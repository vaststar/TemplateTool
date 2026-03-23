#include "UIViewBase/include/UIViewController.h"

#include <AppContext/AppContext.h>
#include <UIManager/IUIManagerProvider.h>
#include <TranslatorManager/ITranslatorManager.h>
#include <UIEventBus/IUIEventBus.h>
#include <UIResourceLoaderManager/IUIResourceLoaderManager.h>

#include "LoggerDefine/LoggerDefine.h"

UIViewController::UIViewController(QObject* parent)
    : UIAppCore::UIController(parent)
{
}

void UIViewController::initializeController(QPointer<AppContext> appContext)
{
    if (getAppContext())
    {
        UIVIEW_LOG_WARN("Controller " << getControllerName().toStdString() << " has been initialized already.");
        return;
    }

    if (appContext.isNull())
    {
        UIVIEW_LOG_ERROR("Failed to initialize Controller " << getControllerName().toStdString() << ": AppContext is null.");
        return;
    }

    UIVIEW_LOG_DEBUG("start initialize Controller: " << getControllerName().toStdString());
    mAppContext = appContext;

    // Listen for language changes (all controllers automatically get this)
    if (auto translatorMgr = appContext->getManagerProvider()->getTranslatorManager())
    {
        connect(translatorMgr, &UIManager::ITranslatorManager::languageChanged, this, [this]() {
            UIVIEW_LOG_DEBUG("receive language change in Controller: " << getControllerName().toStdString());
            onLanguageChanged();
            UIVIEW_LOG_DEBUG("finish handling language change in Controller: " << getControllerName().toStdString());
            emit languageChanged();
            UIVIEW_LOG_DEBUG("send languageChanged signal in Controller: " << getControllerName().toStdString());
        });
    }

    // Listen for theme changes (all controllers automatically get this)
    if (auto resourceLoaderMgr = appContext->getManagerProvider()->getUIResourceLoaderManager())
    {
        connect(resourceLoaderMgr, &UIResource::IUIResourceLoaderManager::themeChanged, this, [this]() {
            UIVIEW_LOG_DEBUG("receive theme change in Controller: " << getControllerName().toStdString());
            onThemeChanged();
            UIVIEW_LOG_DEBUG("finish handling theme change in Controller: " << getControllerName().toStdString());
            emit themeChanged();
            UIVIEW_LOG_DEBUG("send themeChanged signal in Controller: " << getControllerName().toStdString());
        });
    }

    init();
    emit controllerInitialized();
    UIVIEW_LOG_DEBUG("finish initialize Controller: " << getControllerName().toStdString());
}

QPointer<AppContext> UIViewController::getAppContext() const
{
    return mAppContext;
}

void UIViewController::setupController(UIViewController* controller)
{
    if (!controller)
    {
        UIVIEW_LOG_WARN("setupController: controller is null");
        return;
    }

    auto appContext = getAppContext();
    if (!appContext)
    {
        UIVIEW_LOG_WARN("setupController: appContext is null");
        return;
    }

    UIVIEW_LOG_DEBUG("setupController start, from: " << getControllerName().toStdString()
        << ", target: " << controller->getControllerName().toStdString());

    // Call subclass hook for custom logic
    onSetupController(controller);

    // Common initialization
    controller->initializeController(appContext);

    UIVIEW_LOG_DEBUG("setupController finish, target: " << controller->getControllerName().toStdString());
}

void UIViewController::onSetupController(UIViewController* controller)
{
    // Default empty implementation, subclasses override as needed
    Q_UNUSED(controller);
}

void UIViewController::onLanguageChanged()
{
    // Default: no-op. Subclasses override to refresh localized data.
}

void UIViewController::onThemeChanged()
{
    // Default: no-op. Subclasses override to refresh theme-dependent data.
}

void UIViewController::logInfo(const QString& message)
{
    UIVIEW_LOG_INFO("[" << getControllerName().toStdString() << "] " << message.toStdString());
}

QPointer<UIManager::IUIEventBus> UIViewController::getUIEventBus() const
{
    if (auto ctx = getAppContext())
        return ctx->getManagerProvider()->getUIEventBus();
    return nullptr;
}
