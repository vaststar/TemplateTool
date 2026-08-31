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

    // 1. Load translation
    auto clientInfoViewModel = getViewModelFactory()->createClientInfoViewModelInstance();
    clientInfoViewModel->initViewModel();
    const auto configuredLanguage = clientInfoViewModel->getApplicationLanguage();
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
                clientInfoViewModel->setApplicationLanguage(
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

    // 2. Load theme
    APPUI_LOG_DEBUG("get CurrentTheme: " << static_cast<int>(clientInfoViewModel->getCurrentThemeType()));

    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this]() {
        auto clientInfoViewModel = getViewModelFactory()->createClientInfoViewModelInstance();
        clientInfoViewModel->initViewModel();
        APPUI_LOG_DEBUG("system color scheme changed, CurrentTheme: " << static_cast<int>(clientInfoViewModel->getCurrentThemeType()));
        if (clientInfoViewModel->getCurrentThemeType() == commonHead::viewModels::model::ThemeType::SystemDefault)
        {
            APPUI_LOG_DEBUG("system color scheme changed, notifying theme refresh");
            getResourceLoaderManager()->notifyThemeChanged();
        }
    });

    // 3. Show main window
    QTimer::singleShot(0, this, [this]() {
        APPUI_LOG_DEBUG("will show main window");
        showMainWindow();
    });

    APPUI_LOG_DEBUG(
        "AppUIController application configuration processing finished, address: "
        << this);
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
    if (auto* mainController = UIUtilities::QmlWindowPropertyResolver::resolveObjectAs<UIViewController>(
            win, "controller"))
    {
        APPUI_LOG_DEBUG("MainWindow.qml load done, will initialize its root controller");
        setupController(mainController);
        APPUI_LOG_DEBUG("MainWindow root controller initialization done");
    }

    // First placement: fit into the screen's available area, then center.
    UIUtilities::WindowGeometry::clampIntoScreen(win.data());
    UIUtilities::WindowGeometry::centerOnScreen(win.data());

    win->show();
    APPUI_LOG_DEBUG(
        "AppUIController main window loading finished, address: " << this);
}
