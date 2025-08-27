#include "ThemeManager.h"

#include <map>
#include <QLocale>
#include <QTranslator>

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ResourceLoader/IResourceLoader.h>

#include <UICore/CoreApplication.h>
#include <UICore/CoreQmlEngine.h>
#include <UIDataStruct/UIDataUtils.h>
#include <UIResourceLoader/UIResourceLoader.h>
#include <UIResourceColorLoader/UIResourceColorLoader.h>
#include <UIResourceStringLoader/UIResourceStringLoader.h>

#include "LoggerDefine.h"

namespace UIManager{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start Impl Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class ThemeManager::Impl
{
public:
    Impl(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework);

    const QPointer<UICore::CoreApplication> mApplication;
    const QPointer<UICore::CoreQmlEngine> mQmlEngine;
    const commonHead::ICommonHeadFrameworkWPtr mCommonheadFramework;

    std::shared_ptr<commonHead::IResourceLoader> getResourceLoader() const;
};

ThemeManager::Impl::Impl(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
    : mApplication(application)
    , mQmlEngine(qmlEngine)
    , mCommonheadFramework(commonheadFramework)
{
}

std::shared_ptr<commonHead::IResourceLoader> ThemeManager::Impl::getResourceLoader() const
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

ThemeManager::ThemeManager(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
    : mImpl(std::make_unique<ThemeManager::Impl>(application, qmlEngine, commonheadFramework))
{
    UIManager_LOG_INFO("create ThemeManager: " << this);
    registerTypes();
}

ThemeManager::~ThemeManager()
{

}

void ThemeManager::registerTypes()
{
    UIManager_LOG_DEBUG("register ResourceLoader stuff");
    UIResource::UIResourceLoader::registerUIResourceLoader(mImpl->mCommonheadFramework);

    UIManager_LOG_INFO("register ThemeManager: " << this);
    mImpl->mQmlEngine->rootContext()->setContextProperty("ThemeManager", this);
}

QColor ThemeManager::getUIColor(UIColorToken::ColorToken colorEnum, UIColorState::ColorState state)
{
    if (auto resourceLoader = mImpl->getResourceLoader())
    {
        auto vmColorToken = UIResource::UIResourceColorLoader::convertUIColorTokenToVMColorToken(colorEnum);
        auto vmColorState = UIResource::UIResourceColorLoader::convertUIColorStateToVMColorState(state);
        auto vmColor = resourceLoader->getColor(vmColorToken, vmColorState);
        return QColor(vmColor.r, vmColor.g, vmColor.b, static_cast<int>(255*vmColor.a)) ;
    }
    UIManager_LOG_WARN("no resourceLoader");
    return QColor();
}

QFont ThemeManager::getUIFont(UIElementData::UIFontSize size, UIElementData::UIFontWeight weight, bool isItalic, UIElementData::UIFontFamily family)
{
    if (auto resourceLoader = mImpl->getResourceLoader())
    {
        commonHead::model::FontFamily vmFontFamily = UIDataUtils::convertUIFontFamilyToVMFontFamily(family);
        commonHead::model::FontSize vmFontSize = UIDataUtils::convertUIFontSizeToVMFontSize(size);
        commonHead::model::FontWeight vmFontWeight = UIDataUtils::convertUIFontWeightToVMFontWeight(weight);
        auto vmFont = resourceLoader->getFont(vmFontFamily, vmFontSize, vmFontWeight, isItalic);
        return QFont(QString::fromStdString(vmFont.fontFamily), vmFont.fontSize, vmFont.fontWeight, isItalic);
    }
    UIManager_LOG_WARN("no resourceLoader");
    return QFont();
}

QString ThemeManager::getNonLocalizedString(UIStringToken::NonLocalizedString stringId)
{
    if (auto resourceLoader = mImpl->getResourceLoader())
    {
        return resourceLoader->getNonLocalizedString(UIResource::UIResourceStringLoader::convertUINonLocalizedStringToVMNonLocalizedString(stringId)).c_str();
    }
    UIManager_LOG_WARN("no resourceLoader");
    return {};
}

QString ThemeManager::getLocalizedString(UIStringToken::LocalizedString stringId)
{
    if (auto resourceLoader = mImpl->getResourceLoader())
    {
        return resourceLoader->getLocalizedString(UIResource::UIResourceStringLoader::convertUILocalizedStringToVMLocalizedString(stringId)).c_str();
    }
    UIManager_LOG_WARN("no resourceLoader");
    return {};
}

QString ThemeManager::getLocalizedStringWithParams(UIStringToken::LocalizedStringWithParam stringId, const std::initializer_list<std::string>& params)
{
    if (auto resourceLoader = mImpl->getResourceLoader())
    {
        return resourceLoader->getLocalizedStringWithParams(UIResource::UIResourceStringLoader::convertUILocalizedStringParamToVMLocalizedStringParam(stringId), params).c_str();
    }
    UIManager_LOG_WARN("no resourceLoader");
    return {};
}
}
