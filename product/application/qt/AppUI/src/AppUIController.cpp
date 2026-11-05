#include "AppUIController.h"

#include <commonhead/viewmodels/AppUIViewModel/IAppUIViewModel.h>
#include <commonhead/viewmodels/ClientInfoViewModel/IClientInfoViewModel.h>
#include <commonhead/viewmodels/ClientInfoViewModel/ClientInfoModel.h>

#include <TranslatorManager/UILanguage.h>
#include <commonhead/viewmodels/ViewModelFactory/IViewModelFactory.h>
#include <UIFabrication/IUIViewFactory.h>
#include <UIManager/IUIManagerProvider.h>
#include <UIQmlUtilities/QmlWindowPropertyResolver.h>
#include <AppContext/AppContext.h>
#include <TranslatorManager/ITranslatorManager.h>
#include <UIResourceLoaderManager/IUIResourceLoaderManager.h>

#include <QGuiApplication>
#include <QMetaEnum>
#include <QScopedValueRollback>
#include <QStyleHints>
#include <QTimer>

#include "LoggerDefine.h"
#include "UIWindowUtilities/WindowGeometry.h"
#include <UIViewModelSignalBridge/emitters/AppUIViewModelEmitter.h>
#include "UIIPCServerHelper.h"
#include "UIStabilityMonitor.h"
#include "UIScreenMonitor.h"


namespace {
// QML resource paths used by this controller.
const QString kMainWindowQml = QStringLiteral("UIView/MainWindowSuite/MainWindow/qml/MainWindow.qml");
}

AppUIController::AppUIController(QObject* parent)
    : UIViewController(parent)
{
    APPUI_LOG_DEBUG("AppUIController constructed, address: " << this);
}

AppUIController::~AppUIController()
{
    APPUI_LOG_DEBUG("AppUIController destroying, address: " << this);
}

void AppUIController::start(QPointer<AppContext> appContext)
{
    APPUI_LOG_DEBUG(
        "AppUIController startup started, address: " << this);

    initializeController(appContext);

    APPUI_LOG_DEBUG(
        "AppUIController startup finished, address: " << this);
}

void AppUIController::init()
{
    auto appContext = getAppContext();

    // Create ViewModel and connect signals
    mAppUIViewModel = getViewModelFactory()->createAppUIViewModelInstance();
    mAppUIViewModelEmitter = std::make_shared<UIViewModelSignalBridge::AppUIViewModelEmitter>();
    mAppUIViewModel->registerCallback(mAppUIViewModelEmitter);

    QObject::connect(mAppUIViewModelEmitter.get(), &UIViewModelSignalBridge::AppUIViewModelEmitter::signals_onAppConfigInitialized,
                     this, &AppUIController::onAppConfigInitialized);

    // Start IPC server and stability monitor
    mIPCServerHelper = std::make_unique<UIIPCServerHelper>(appContext, this);
    mIPCServerHelper->start();

    mStabilityMonitor = std::make_unique<UIStabilityMonitor>(appContext, this);
    mStabilityMonitor->start();

    // Watch for runtime screen/DPI changes and broadcast UIScreenChangedEvent so
    // window controllers can re-fit their windows into the available area.
    mScreenMonitor = std::make_unique<UIScreenMonitor>(appContext, this);
    mScreenMonitor->start();

    // Defer application initialization to the first event loop iteration,
    // ensuring QApplication::exec() is running before async callbacks fire.
    QTimer::singleShot(0, this, [this]() {
        mAppUIViewModel->initViewModel();
    });
}

void AppUIController::onAppConfigInitialized()
{
    APPUI_LOG_DEBUG(
        "AppUIController application configuration processing started, address: "
        << this);

    mClientInfoViewModel = getViewModelFactory()->createClientInfoViewModelInstance();
    mClientInfoViewModel->initViewModel();
    applyConfiguredLanguage();
    initializeTheme();

    QTimer::singleShot(0, this, &AppUIController::showMainWindow);

    APPUI_LOG_DEBUG(
        "AppUIController application configuration processing finished, address: "
        << this);
}

void AppUIController::applyConfiguredLanguage()
{
    const auto configuredLanguage = mClientInfoViewModel->getApplicationLanguage();
    APPUI_LOG_DEBUG("get language: " << static_cast<int>(configuredLanguage));

    if (auto translatorManager = getTranslatorManager())
    {
        const auto result = translatorManager->loadTranslation(
            UILanguage::convertFromViewModel(configuredLanguage));
        if (!UIManager::isTranslationLoadSuccessful(result))
        {
            APPUI_LOG_WARN("configured language could not be applied, languageType:"
                << static_cast<int>(configuredLanguage)
                << ", result:" << static_cast<int>(result)
                << "; falling back to English");

            const auto fallbackResult = translatorManager->loadTranslation(
                UILanguage::LanguageType::LanguageType_ENGLISH);
            if (UIManager::isTranslationLoadSuccessful(fallbackResult))
            {
                mClientInfoViewModel->setApplicationLanguage(
                    commonHead::viewModels::model::LanguageType::ENGLISH);
            }
            else
            {
                APPUI_LOG_ERROR("English translation fallback failed, result:"
                    << static_cast<int>(fallbackResult));
            }
        }
    }
    else
    {
        APPUI_LOG_ERROR("TranslatorManager is unavailable");
    }
}

void AppUIController::initializeTheme()
{
    syncQtColorScheme();
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
        this, &AppUIController::onQtColorSchemeChanged, Qt::UniqueConnection);
}

void AppUIController::onThemeChanged()
{
    syncQtColorScheme();
}

void AppUIController::syncQtColorScheme()
{
    if (!mClientInfoViewModel)
        return;

    using ThemeType = commonHead::viewModels::model::ThemeType;
    const auto theme = mClientInfoViewModel->getCurrentThemeType();
    auto requested = Qt::ColorScheme::Unknown;
    switch (theme)
    {
    case ThemeType::Light:
        requested = Qt::ColorScheme::Light;
        break;
    case ThemeType::Dark:
        requested = Qt::ColorScheme::Dark;
        break;
    case ThemeType::SystemDefault:
        break;
    }

    // Compare requests, not the effective scheme: Unknown restores system tracking.
    if (mRequestedColorScheme == requested)
        return;
    mRequestedColorScheme = requested;

    auto* hints = QGuiApplication::styleHints();
    const auto before = hints->colorScheme();
    {
        // Qt can emit colorSchemeChanged synchronously while applying this request.
        QScopedValueRollback<bool> applying(mApplyingQtColorScheme, true);
        hints->setColorScheme(requested);
    }

    const auto schemes = QMetaEnum::fromType<Qt::ColorScheme>();
    APPUI_LOG_DEBUG("Qt color scheme sync: appTheme=" << static_cast<int>(theme)
        << ", requested=" << schemes.valueToKey(static_cast<int>(requested))
        << ", before=" << schemes.valueToKey(static_cast<int>(before))
        << ", observed=" << schemes.valueToKey(static_cast<int>(hints->colorScheme())));
}

void AppUIController::onQtColorSchemeChanged(Qt::ColorScheme scheme)
{
    const auto theme = mClientInfoViewModel->getCurrentThemeType();
    const auto schemes = QMetaEnum::fromType<Qt::ColorScheme>();
    APPUI_LOG_DEBUG("Qt color scheme changed: scheme=" << schemes.valueToKey(static_cast<int>(scheme))
        << ", appTheme=" << static_cast<int>(theme)
        << ", applyingAppTheme=" << mApplyingQtColorScheme);

    if (mApplyingQtColorScheme || theme != commonHead::viewModels::model::ThemeType::SystemDefault)
        return;

    if (auto resourceLoaderManager = getResourceLoaderManager())
    {
        APPUI_LOG_DEBUG("system color scheme changed, notifying theme refresh");
        resourceLoaderManager->notifyThemeChanged();
    }
}

void AppUIController::showMainWindow()
{
    APPUI_LOG_DEBUG(
        "AppUIController main window loading started, address: " << this);

    auto win = getViewFactory()->createQmlWindow(
        kMainWindowQml);
    if (!win)
    {
        APPUI_LOG_WARN(
            "AppUIController main window loading failed: unable to create QML window"
            ", address: "
            << this);
        return;
    }

    auto* mainController =
        UIUtilities::QmlWindowPropertyResolver::resolveObjectAs<UIViewController>(
            win, "controller");
    if (mainController)
    {
        APPUI_LOG_DEBUG("MainWindow.qml load done, will initialize its root controller");
        setupController(mainController);
        APPUI_LOG_DEBUG("MainWindow root controller initialization done");
    }
    else
    {
        APPUI_LOG_ERROR(
            "Main window root controller is unavailable, mainWindow address: "
            << win.data());
    }

    // First placement: fit into the screen's available area, then center.
    UIUtilities::WindowGeometry::clampIntoScreen(win.data());
    UIUtilities::WindowGeometry::centerOnScreen(win.data());

    win->show();
    APPUI_LOG_DEBUG(
        "AppUIController main window loading finished, address: " << this);

    if (mainController)
    {
        APPUI_LOG_INFO(
            "Application ready, mainWindow address: "
            << win.data()
            << ", rootController address: "
            << mainController);
    }
}
