#include <UIManager/ThemeManager.h>

#include <map>
#include <QLocale>
#include <QTranslator>

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ResourceLoader/IResourceLoader.h>

#include <UICore/CoreApplication.h>
#include <UICore/CoreQmlEngine.h>
#include <UIDataStruct/UIDataUtils.h>

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
    : QObject(nullptr)
    , mImpl(std::make_unique<ThemeManager::Impl>(application, qmlEngine, commonheadFramework))
{
    UIManager_LOG_DEBUG("");
}

ThemeManager::~ThemeManager()
{

}

QColor ThemeManager::getUIColor(UIElementData::UIColorEnum colorEnum, UIElementData::UIColorState state)
{
    if (auto resourceLoader = mImpl->getResourceLoader())
    {
        commonHead::model::ColorItem vmColorItem = UIDataUtils::convertUIColorEnumToVMColorItem(colorEnum);
        commonHead::model::ColorItemState vmColorItemState = UIDataUtils::convertUIColorStateToVMColorItemState(state);
        auto vmColor = resourceLoader->getColor(vmColorItem, vmColorItemState);
        return QColor(vmColor.r, vmColor.g, vmColor.b, vmColor.a) ;
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

QString ThemeManager::getLocalizedString(UIStrings::LocalizedString stringId)
{
    if (auto resourceLoader = mImpl->getResourceLoader())
    {

        return resourceLoader->getLocalizedStringWithParams(commonHead::model::LocalizedStringWithParam::TestParm,{"ceshi"}).c_str();
        return resourceLoader->getLocalizedString(UIDataUtils::convertUILocalizedStringToVMLocalizedString(stringId)).c_str();
    }
    UIManager_LOG_WARN("no resourceLoader");
    return {};
}

QString ThemeManager::getLocalizedStringWithParams(UIStrings::LocalizedStringWithParam stringId, const std::initializer_list<std::string>& params)
{
    if (auto resourceLoader = mImpl->getResourceLoader())
    {
        return resourceLoader->getLocalizedStringWithParams(UIDataUtils::convertUILocalizedStringParamToVMLocalizedStringParam(stringId), params).c_str();
    }
    UIManager_LOG_WARN("no resourceLoader");
    return {};
}
}
