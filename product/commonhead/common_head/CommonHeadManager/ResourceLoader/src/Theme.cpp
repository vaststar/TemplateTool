#include "Theme.h"

#include <ucf/Services/ClientInfoService/ClientInfoModel.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>

#include "ColorConstant.h"

namespace commonHead{

Theme::Theme(ucf::service::model::ThemeType themeType)
    : mThemeType(themeType)
    , mFontSet(std::make_unique<FontSet>())
    , mColorSet(std::make_unique<ColorSet>())
{
    initFonts();
    initColors();
}

ucf::service::model::ThemeType Theme::getThemeType() const
{
    return mThemeType;
}

model::Font Theme::getFont(model::FontFamily family, model::FontSize size, model::FontWeight weight, bool isItalic) const
{
    if (auto fonts = mFontSet->getFonts(family))
    {
        return fonts->getFont(size, weight, isItalic);
    }
    COMMONHEAD_LOG_WARN("cant find font, family: " << static_cast<int>(family) << ", size: " << static_cast<int>(size) << ", weight: " << static_cast<int>(weight) << ", isItalic: " << isItalic);
    return model::Font();
}

model::Color Theme::getColor(model::ColorItem colorItem, model::ColorItemState state) const
{
    if (auto colors = mColorSet->getColors(colorItem))
    {
        return colors->getColor(state);
    }
    COMMONHEAD_LOG_WARN("cant find colorItem: " << static_cast<int>(colorItem) << ", state: " << static_cast<int>(state));
    return model::Color();
}

void Theme::initFonts()
{
    std::vector<std::shared_ptr<Fonts>> uiFonts;
    uiFonts.emplace_back(std::make_shared<Fonts>(model::FontFamily::FontFamily_SegoeUI));
    uiFonts.emplace_back(std::make_shared<Fonts>(model::FontFamily::FontFamily_Consolas));
    uiFonts.emplace_back(std::make_shared<Fonts>(model::FontFamily::FontFamily_SegoeUIEmoji));
    mFontSet->initFonts(uiFonts);
}

void Theme::initColors()
{
    std::vector<std::shared_ptr<Colors>> uiColors;
    uiColors.emplace_back(std::make_shared<Colors>(model::ColorItem::ColorItem_Button_Primary_Text, model::Color{0,0,0}));
    uiColors.emplace_back(std::make_shared<Colors>(model::ColorItem::ColorItem_Button_Primary_Background, model::Color{0,122, 163}, model::Color{0,160, 209}, model::Color{82,220, 255}, model::Color{41,41, 41}, model::Color{0,122, 163}, model::Color{0,122, 163}));
    uiColors.emplace_back(std::make_shared<Colors>(model::ColorItem::ColorItem_Button_Primary_Border, model::Color{0,0,255}));
    mColorSet->initColors(uiColors);
}

}