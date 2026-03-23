#include "UIManagerProvider.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

#include <UIAppCore/UIApplication.h>
#include <UIAppCore/UIQmlEngine.h>

#include <UIResourceLoaderManager/IUIResourceLoaderManager.h>
#include <TranslatorManager/ITranslatorManager.h>
#include <UIEventBus/IUIEventBus.h>

#include "LoggerDefine.h"

namespace UIManager{
std::unique_ptr<IUIManagerProvider> IUIManagerProvider::createInstance(QPointer<UIAppCore::UIApplication> application, QPointer<UIAppCore::UIQmlEngine> qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
{
    return std::make_unique<UIManagerProvider>(application, qmlEngine, commonheadFramework);
}

UIManagerProvider::UIManagerProvider(QPointer<UIAppCore::UIApplication> application, QPointer<UIAppCore::UIQmlEngine> qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
    : mTranslatorManager(ITranslatorManager::createInstance(application, qmlEngine))
    , mUIResourceLoaderManager(UIResource::IUIResourceLoaderManager::createInstance(application, qmlEngine, commonheadFramework))
    , mUIEventBus(IUIEventBus::createInstance(this))
{
    UIManager_LOG_DEBUG("create UIManagerProvider: " << this);
}

UIManagerProvider::~UIManagerProvider()
{

}

QPointer<ITranslatorManager> UIManagerProvider::getTranslatorManager() const
{
    return mTranslatorManager.get();
}

QPointer<UIResource::IUIResourceLoaderManager> UIManagerProvider::getUIResourceLoaderManager() const
{
    return mUIResourceLoaderManager.get();
}

QPointer<IUIEventBus> UIManagerProvider::getUIEventBus() const
{
    return mUIEventBus.get();
}
}
