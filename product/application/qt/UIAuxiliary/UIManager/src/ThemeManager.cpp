#include <UIManager/ThemeManager.h>

#include <map>
#include <QLocale>
#include <QTranslator>

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ResourceLoader/IResourceLoader.h>

#include <UICore/CoreApplication.h>
#include <UICore/CoreQmlEngine.h>

#include "LoggerDefine.h"
#include "ColorConstant.h"

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

    QPointer<UIData::UIFontSet> getFontSet() const;
    QPointer<UIData::UIColorSet> getColorSet() const;
private:
    void initFontSet();
    void initColorSet();
private:
    std::unique_ptr<UIData::UIFontSet> mFontSet;
    std::unique_ptr<UIData::UIColorSet> mColorSet;
};

ThemeManager::Impl::Impl(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine, commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
    : mApplication(application)
    , mQmlEngine(qmlEngine)
    , mCommonheadFramework(commonheadFramework)
    , mFontSet(std::make_unique<UIData::UIFontSet>())
    , mColorSet(std::make_unique<UIData::UIColorSet>())
{
    initFontSet();
    initColorSet();
}

QPointer<UIData::UIFontSet> ThemeManager::Impl::getFontSet() const
{
    return mFontSet.get();
}

QPointer<UIData::UIColorSet> ThemeManager::Impl::getColorSet() const
{
    return mColorSet.get();
}

void ThemeManager::Impl::initFontSet()
{
    std::vector<std::shared_ptr<UIData::UIFont>> uiFonts;
    uiFonts.emplace_back(std::make_shared<UIData::UIFont>(UIData::UIFont::UIFontFamily::UIFontFamily_SegoeUI));
    uiFonts.emplace_back(std::make_shared<UIData::UIFont>(UIData::UIFont::UIFontFamily::UIFontFamily_Consolas));
    uiFonts.emplace_back(std::make_shared<UIData::UIFont>(UIData::UIFont::UIFontFamily::UIFontFamily_SegoeUIEmoji));
    mFontSet->initFonts(uiFonts);
}

void ThemeManager::Impl::initColorSet()
{
    std::vector<std::shared_ptr<UIData::UIColors>> uiColors;
    uiColors.emplace_back(std::make_shared<UIData::UIColors>(UIData::UIColors::UIColorsEnum::UIColorsEnum_Button_Primary_Text, QColor(0,0,0)));
    uiColors.emplace_back(std::make_shared<UIData::UIColors>(UIData::UIColors::UIColorsEnum::UIColorsEnum_Button_Primary_Background, ColorConstant::Blue60, ColorConstant::Blue50, ColorConstant::Blue30, ColorConstant::Gray90, ColorConstant::Blue60, ColorConstant::Blue60));
    uiColors.emplace_back(std::make_shared<UIData::UIColors>(UIData::UIColors::UIColorsEnum::UIColorsEnum_Button_Primary_Border, QColor(0,0,255)));
    mColorSet->initColors(uiColors);
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

// void initialize();
void ThemeManager::test()
{
    UIManager_LOG_DEBUG("");
}

QColor ThemeManager::getUIColor(UIData::UIColors::UIColorsEnum colorEnum, UIData::UIColors::UIColorState state)
{
    switch (colorEnum)
    {
    case /* constant-expression */:
        /* code */
        break;
    
    default:
        break;
    }

    
    switch (state)
    {
    case UIData::UIColors::UIColorState::UIColorState_Normal:
        /* code */
        break;
    
    default:
        break;
    }
    mImpl->mCommonheadFramework->getResourceLoader()->getColor();

    // if (auto uiColors = mImpl->getColorSet()->getUIColors(colorEnum))
    // {
    //     return uiColors->getColor(state);
    // }
    // return QColor(0,0,0);
}

QFont ThemeManager::getUIFont(UIData::UIFont::UIFontSize size, UIData::UIFont::UIFontWeight weight, bool isItalic, UIData::UIFont::UIFontFamily family)
{
    if (auto uiFonts = mImpl->getFontSet()->getUIFont(family))
    {
        return uiFonts->getFont(size, weight, isItalic);
    }
    return QFont("Segoe UI", 14, QFont::Normal, false);
}
}
