#include "FontSet.h"

#include <map>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>

namespace commonHead{

static const std::map<model::FontFamily, std::string> FontFamilyMap = {
    {model::FontFamily::FontFamily_SegoeUI, "SegoeUI "},
    {model::FontFamily::FontFamily_Consolas, "Consolas"},
    {model::FontFamily::FontFamily_SegoeUIEmoji, "SegoeUI  Emoji"}
};

Fonts::Fonts(model::FontFamily family)
    : mFontFamly(family)
{
    if (auto iter = FontFamilyMap.find(family); iter != FontFamilyMap.end())
    {
        mFontFamilyName = iter->second;
    }
    else
    {
        mFontFamilyName = "Segoe ";
    }
}

model::Font Fonts::getFont(model::FontSize size, model::FontWeight weight, bool isItalic)
{
    int fontSize = 12;
    switch (size)
    {
    case model::FontSize::FontSize_ExtraSmall:
        fontSize = 12;
        break;
    case model::FontSize::FontSize_Small:
        fontSize = 14;
        break;
    case model::FontSize::FontSize_MediumSmall:
        fontSize = 16;
        break;
    case model::FontSize::FontSize_Medium:
        fontSize = 20;
        break;
    case model::FontSize::FontSize_Large:
        fontSize = 26;
        break;
    case model::FontSize::FontSize_ExtraLarge:
        fontSize = 40;
        break;
    default:
        break;
    }

    int fontWeight = 400;
    switch (weight)
    {
    case model::FontWeight::FontWeight_Thin:
        fontWeight = 100;
        break;
    case model::FontWeight::FontWeight_ExtraLight:
        fontWeight = 200;
        break;
    case model::FontWeight::FontWeight_Light:
        fontWeight = 300;
        break;
    case model::FontWeight::FontWeight_Normal:
        fontWeight = 400;
        break;
    case model::FontWeight::FontWeight_Medium:
        fontWeight = 500;
        break;
    case model::FontWeight::FontWeight_DemiBold:
        fontWeight = 600;
        break;
    case model::FontWeight::FontWeight_Bold:
        fontWeight = 700;
        break;
    case model::FontWeight::FontWeight_ExtraBold:
        fontWeight = 800;
        break;
    case model::FontWeight::FontWeight_Black:
        fontWeight = 900;
        break;
    default:
        break;
    }
    return model::Font{mFontFamilyName, fontSize, fontWeight, isItalic};
}

model::FontFamily Fonts::getFontFamily()
{
    return mFontFamly;
}

FontSet::FontSet()
{

}

void FontSet::initFonts(const std::vector<std::shared_ptr<Fonts>>& fonts)
{
    mFonts = fonts;
}

std::shared_ptr<Fonts> FontSet::getFonts(model::FontFamily family)
{
    auto iter = std::find_if(mFonts.cbegin(), mFonts.cend(), [family](const auto uFont){
        return family == uFont->getFontFamily();
    });
    if (iter != mFonts.cend())
    {
        return *iter;
    }
    COMMONHEAD_LOG_DEBUG("can't find this ui family: " << static_cast<int>(family));
    return nullptr;
}
}