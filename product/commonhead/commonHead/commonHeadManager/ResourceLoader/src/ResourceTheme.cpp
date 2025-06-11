#include "ResourceTheme.h"

#include <ucf/Services/ClientInfoService/ClientInfoModel.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>

#include "ColorConstant.h"
#include "ColorBuilder.h"

namespace commonHead{

ResourceTheme::ResourceTheme(ucf::service::model::ThemeType themeType)
    : mThemeType(themeType)
    , mFontSet(std::make_unique<FontSet>())
    , mColorSet(std::make_unique<ColorSet>())
{
    initFonts();
    initColors();
}

ucf::service::model::ThemeType ResourceTheme::getThemeType() const
{
    return mThemeType;
}

model::Font ResourceTheme::getFont(model::FontFamily family, model::FontSize size, model::FontWeight weight, bool isItalic) const
{
    if (auto fonts = mFontSet->getFonts(family))
    {
        return fonts->getFont(size, weight, isItalic);
    }
    COMMONHEAD_LOG_WARN("cant find font, family: " << static_cast<int>(family) << ", size: " << static_cast<int>(size) << ", weight: " << static_cast<int>(weight) << ", isItalic: " << isItalic);
    return model::Font();
}

model::Color ResourceTheme::getColor(model::ColorItem colorItem, model::ColorItemState state) const
{
    if (auto colors = mColorSet->getColors(colorItem))
    {
        return colors->getColor(state);
    }
    COMMONHEAD_LOG_WARN("cant find colorItem: " << static_cast<int>(colorItem) << ", state: " << static_cast<int>(state));
    return model::Color();
}

void ResourceTheme::initFonts()
{
    std::vector<std::shared_ptr<Fonts>> uiFonts;
    uiFonts.emplace_back(std::make_shared<Fonts>(model::FontFamily::FontFamily_SegoeUI));
    uiFonts.emplace_back(std::make_shared<Fonts>(model::FontFamily::FontFamily_Consolas));
    uiFonts.emplace_back(std::make_shared<Fonts>(model::FontFamily::FontFamily_SegoeUIEmoji));
    mFontSet->initFonts(uiFonts);
}

void ResourceTheme::initColors()
{
    mColorSet->initColors(ColorBuilder().buildColors(mThemeType));
}

}