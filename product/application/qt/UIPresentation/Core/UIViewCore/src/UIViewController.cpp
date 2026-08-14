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
    if (getAppContext())
    {
        UIViewCore_LOG_WARN("Controller " << getControllerName().toStdString() << " has been initialized already.");
        return;
    }

    if (appContext.isNull())
    {
        UIViewCore_LOG_ERROR("Failed to initialize Controller " << getControllerName().toStdString() << ": AppContext is null.");
        return;
    }

    UIViewCore_LOG_DEBUG("start initialize Controller: " << getControllerName().toStdString());
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
    UIViewCore_LOG_DEBUG("finish initialize Controller: " << getControllerName().toStdString());
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
        UIViewCore_LOG_WARN("setupController: controller is null");
        return;
    }

    if (controller->isInitialized())
    {
        UIViewCore_LOG_DEBUG("setupController: controller "
            << controller->getControllerName().toStdString()
            << " has already been initialized; skipping setup");
        return;
    }

    auto appContext = getAppContext();
    if (!appContext)
    {
        UIViewCore_LOG_WARN("setupController: appContext is null");
        return;
    }

    UIViewCore_LOG_DEBUG("setupController start, from: " << getControllerName().toStdString()
        << ", target: " << controller->getControllerName().toStdString());

    onSetupController(controller);
    controller->initializeController(appContext);

    UIViewCore_LOG_DEBUG("setupController finish, target: " << controller->getControllerName().toStdString());
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
