#include "UIResourceLoaderManager.h"

#include <map>

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ResourceLoader/IResourceLoader.h>

#include <UICore/CoreApplication.h>
#include <UICore/CoreQmlEngine.h>
#include <UIResourceLoader/UIResourceLoader.h>
#include <UIResourceColorLoader/UIResourceColorLoader.h>
#include <UIResourceStringLoader/UIResourceStringLoader.h>
#include <UIResourceFontLoader/UIResourceFontLoader.h>

#include "LoggerDefine.h"

namespace UIResource{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<IUIResourceLoaderManager> IUIResourceLoaderManager::createInstance(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
{
    return std::make_unique<UIResourceLoaderManager>(application, qmlEngine, commonheadFramework);
}

class UIResourceLoaderManager::Impl
{
public:
    Impl(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework);

    const QPointer<UICore::CoreApplication> mApplication;
    const QPointer<UICore::CoreQmlEngine> mQmlEngine;
    const commonHead::ICommonHeadFrameworkWPtr mCommonheadFramework;

    std::shared_ptr<commonHead::IResourceLoader> getResourceLoader() const;
};

UIResourceLoaderManager::Impl::Impl(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
    : mApplication(application)
    , mQmlEngine(qmlEngine)
    , mCommonheadFramework(commonheadFramework)
{
}

std::shared_ptr<commonHead::IResourceLoader> UIResourceLoaderManager::Impl::getResourceLoader() const
{
    if (auto commonHeadFramework = mCommonheadFramework.lock())
    {
        return commonHeadFramework->getResourceLoader();
    }
    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

UIResourceLoaderManager::UIResourceLoaderManager(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
    : mImpl(std::make_unique<UIResourceLoaderManager::Impl>(application, qmlEngine, commonheadFramework))
{
    UIResourceLoaderManager_LOG_INFO("create UIResourceLoaderManager: " << this);
    registerTypes();
}

UIResourceLoaderManager::~UIResourceLoaderManager()
{

}

void UIResourceLoaderManager::registerTypes()
{
    UIResourceLoaderManager_LOG_DEBUG("register ResourceLoader stuff");
    UIResource::UIResourceLoader::registerUIResourceLoader(mImpl->mCommonheadFramework);

    UIResourceLoaderManager_LOG_INFO("register UIResourceLoaderManager: " << this);
    mImpl->mQmlEngine->rootContext()->setContextProperty("UIResourceLoaderManager", this);
}

QColor UIResourceLoaderManager::getUIColor(UIColorToken::ColorToken colorEnum, UIColorState::ColorState state)
{
    if (auto resourceLoader = mImpl->getResourceLoader())
    {
        auto vmColorToken = UIResource::UIResourceColorLoader::convertUIColorTokenToVMColorToken(colorEnum);
        auto vmColorState = UIResource::UIResourceColorLoader::convertUIColorStateToVMColorState(state);
        auto vmColor = resourceLoader->getColor(vmColorToken, vmColorState);
        return QColor(vmColor.r, vmColor.g, vmColor.b, static_cast<int>(255*vmColor.a)) ;
    }
    UIResourceLoaderManager_LOG_WARN("no resourceLoader");
    return QColor();
}

QFont UIResourceLoaderManager::getUIFont(UIFontToken::FontToken fontToken)
{
    if (auto resourceLoader = mImpl->getResourceLoader())
    {
        auto vmFontToken = UIResource::UIResourceFontLoader::convertUIFontTokenToVMFontToken(fontToken);
        auto vmFont = resourceLoader->getFont(vmFontToken);
        return QFont(QString::fromStdString(vmFont.fontFamily), vmFont.fontSize, vmFont.fontWeight, vmFont.isItalic);
    }
    UIResourceLoaderManager_LOG_WARN("no resourceLoader");
    return QFont();
}

QString UIResourceLoaderManager::getNonLocalizedString(UIStringToken::NonLocalizedString stringId)
{
    if (auto resourceLoader = mImpl->getResourceLoader())
    {
        return resourceLoader->getNonLocalizedString(UIResource::UIResourceStringLoader::convertUINonLocalizedStringToVMNonLocalizedString(stringId)).c_str();
    }
    UIResourceLoaderManager_LOG_WARN("no resourceLoader");
    return {};
}

QString UIResourceLoaderManager::getLocalizedString(UIStringToken::LocalizedString stringId)
{
    if (auto resourceLoader = mImpl->getResourceLoader())
    {
        return resourceLoader->getLocalizedString(UIResource::UIResourceStringLoader::convertUILocalizedStringToVMLocalizedString(stringId)).c_str();
    }
    UIResourceLoaderManager_LOG_WARN("no resourceLoader");
    return {};
}

QString UIResourceLoaderManager::getLocalizedStringWithParams(UIStringToken::LocalizedStringWithParam stringId, const std::initializer_list<std::string>& params)
{
    if (auto resourceLoader = mImpl->getResourceLoader())
    {
        return resourceLoader->getLocalizedStringWithParams(UIResource::UIResourceStringLoader::convertUILocalizedStringParamToVMLocalizedStringParam(stringId), params).c_str();
    }
    UIResourceLoaderManager_LOG_WARN("no resourceLoader");
    return {};
}
}
