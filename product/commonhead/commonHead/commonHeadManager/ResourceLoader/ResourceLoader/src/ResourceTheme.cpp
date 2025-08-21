#include "ResourceTheme.h"

#include <ucf/Services/ClientInfoService/ClientInfoModel.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>

namespace commonHead{

ResourceTheme::ResourceTheme(ucf::service::model::ThemeType themeType)
    : mThemeType(themeType)
    , mFontSet(std::make_unique<FontSet>())
{
    initFonts();
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

void ResourceTheme::initFonts()
{
    std::vector<std::shared_ptr<Fonts>> uiFonts;
    uiFonts.emplace_back(std::make_shared<Fonts>(model::FontFamily::FontFamily_SegoeUI));
    uiFonts.emplace_back(std::make_shared<Fonts>(model::FontFamily::FontFamily_Consolas));
    uiFonts.emplace_back(std::make_shared<Fonts>(model::FontFamily::FontFamily_SegoeUIEmoji));
    mFontSet->initFonts(uiFonts);
}
}