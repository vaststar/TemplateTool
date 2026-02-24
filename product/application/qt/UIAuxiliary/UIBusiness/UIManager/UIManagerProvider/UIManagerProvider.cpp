#include "UIManagerProvider.h"

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

#include <UIAppCore/UIApplication.h>
#include <UIAppCore/UIQmlEngine.h>

#include <UIResourceLoaderManager/IUIResourceLoaderManager.h>

#include "LoggerDefine/LoggerDefine.h"
#include "TranslatorManager/TranslatorManager.h"

namespace UIManager{
std::unique_ptr<IUIManagerProvider> IUIManagerProvider::createInstance(QPointer<UIAppCore::UIApplication> application, QPointer<UIAppCore::UIQmlEngine> qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
{
    return std::make_unique<UIManagerProvider>(application, qmlEngine, commonheadFramework);
}

UIManagerProvider::UIManagerProvider(QPointer<UIAppCore::UIApplication> application, QPointer<UIAppCore::UIQmlEngine> qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
    : mTranslatorManager(std::make_unique<TranslatorManager>(application, qmlEngine))
    , mUIResourceLoaderManager(UIResource::IUIResourceLoaderManager::createInstance(application, qmlEngine, commonheadFramework))
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
}
