#include <UIViewCore/UIViewController.h>

#include <AppContext/AppContext.h>
#include <TranslatorManager/ITranslatorManager.h>
#include <UIFabrication/IUIViewFactory.h>
#include <UIManager/IUIManagerProvider.h>
#include <UIResourceLoaderManager/IUIResourceLoaderManager.h>

#include "LoggerDefine.h"

UIViewController::UIViewController(QObject* parent)
    : UIAppCore::UIController(parent)
{
}

bool UIViewController::isInitialized() const
{
    return !mAppContext.isNull();
}

void UIViewController::initializeController(QPointer<AppContext> appContext)
{
    const auto controllerName = getControllerName().toStdString();
    const auto* objectAddress = dynamic_cast<const void*>(this);

    if (isInitialized())
    {
        UIViewCore_LOG_WARN(
            controllerName
            << " initialization skipped: already initialized, address: "
            << objectAddress);
        return;
    }

    if (appContext.isNull())
    {
        UIViewCore_LOG_ERROR(
            controllerName
            << " initialization failed: AppContext is null, address: "
            << objectAddress);
        return;
    }

    UIViewCore_LOG_DEBUG(
        controllerName
        << " initialization started, address: "
        << objectAddress);

    mAppContext = appContext;

    if (auto translatorManager = getTranslatorManager())
    {
        connect(translatorManager, &UIManager::ITranslatorManager::languageChanged, this, [this]() {
            UIViewCore_LOG_DEBUG("receive language change in Controller: " << getControllerName().toStdString());
            onLanguageChanged();
            emit languageChanged();
        });
    }

    if (auto resourceLoaderManager = getResourceLoaderManager())
    {
        connect(resourceLoaderManager, &UIResource::IUIResourceLoaderManager::themeChanged, this, [this]() {
            UIViewCore_LOG_DEBUG("receive theme change in Controller: " << getControllerName().toStdString());
            onThemeChanged();
            emit themeChanged();
        });
    }

    init();
    emit controllerInitialized();

    UIViewCore_LOG_DEBUG(
        controllerName
        << " initialization finished, address: "
        << objectAddress);
}

QPointer<AppContext> UIViewController::getAppContext() const
{
    return mAppContext;
}

std::shared_ptr<commonHead::viewModels::IViewModelFactory> UIViewController::getViewModelFactory() const
{
    if (auto appContext = getAppContext())
    {
        return appContext->getViewModelFactory();
    }
    return {};
}

QPointer<UIFabrication::IUIViewFactory> UIViewController::getViewFactory() const
{
    if (auto appContext = getAppContext())
    {
        return appContext->getViewFactory();
    }
    return nullptr;
}

QPointer<UIManager::IUIManagerProvider> UIViewController::getManagerProvider() const
{
    if (auto appContext = getAppContext())
    {
        return appContext->getManagerProvider();
    }
    return nullptr;
}

QPointer<UIManager::ITranslatorManager> UIViewController::getTranslatorManager() const
{
    if (auto managerProvider = getManagerProvider())
    {
        return managerProvider->getTranslatorManager();
    }
    return nullptr;
}

QPointer<UIResource::IUIResourceLoaderManager> UIViewController::getResourceLoaderManager() const
{
    if (auto managerProvider = getManagerProvider())
    {
        return managerProvider->getUIResourceLoaderManager();
    }
    return nullptr;
}

void UIViewController::setupController(UIViewController* controller)
{
    if (!controller)
    {
        UIViewCore_LOG_WARN(
            "setupController failed: controller is null");
        return;
    }

    const auto* childObjectAddress =
        dynamic_cast<const void*>(controller);
    const auto childControllerName =
        controller->getControllerName().toStdString();

    if (controller->isInitialized())
    {
        UIViewCore_LOG_DEBUG(
            childControllerName
            << " setup skipped: already initialized, address: "
            << childObjectAddress);
        return;
    }

    auto appContext = getAppContext();
    if (!appContext)
    {
        UIViewCore_LOG_WARN(
            getControllerName().toStdString()
            << " failed to set up child controller "
            << childControllerName
            << ": AppContext is null");
        return;
    }

    UIViewCore_LOG_DEBUG(
        getControllerName().toStdString()
        << " setting up child controller "
        << childControllerName
        << ", child address: "
        << childObjectAddress);

    onSetupController(controller);
    controller->initializeController(appContext);
}

void UIViewController::onSetupController(UIViewController* controller)
{
    Q_UNUSED(controller);
}

void UIViewController::onLanguageChanged()
{
}

void UIViewController::onThemeChanged()
{
}

void UIViewController::logInfo(const QString& message)
{
    UIViewCore_LOG_INFO("[" << getControllerName().toStdString() << "] " << message.toStdString());
}

QPointer<UIManager::IUIEventBus> UIViewController::getUIEventBus() const
{
    if (auto managerProvider = getManagerProvider())
    {
        return managerProvider->getUIEventBus();
    }
    return nullptr;
}
