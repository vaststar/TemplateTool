#include "ResourceThemeLoader.h"

#include <ucf/Services/ClientInfoService/ClientInfoModel.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>

#include "FontSet.h"
#include "ColorSet.h"
#include "Theme.h"

namespace commonHead{

ResourceThemeLoader::ResourceThemeLoader()
{

}


model::Font ResourceThemeLoader::getFont(ucf::service::model::ThemeType themeType, model::FontFamily family, model::FontSize size, model::FontWeight weight, bool isItalic) const
{
    if (auto theme = getOrCreateTheme(themeType))
    {
        return theme->getFont(family, size, weight, isItalic);
    }
    COMMONHEAD_LOG_WARN("can't find theme");
    return model::Font();
}

model::Color ResourceThemeLoader::getColor(ucf::service::model::ThemeType themeType, model::ColorItem colorItem, model::ColorItemState state) const
{
    if (auto theme = getOrCreateTheme(themeType))
    {
         return theme->getColor(colorItem, state);
    }
    COMMONHEAD_LOG_WARN("can't find theme");
    return model::Color();
}

std::shared_ptr<Theme> ResourceThemeLoader::getOrCreateTheme(ucf::service::model::ThemeType themeType) const
{
    std::scoped_lock<std::mutex> loc(mThemeMutex);
    auto iter = std::find_if(mThemes.cbegin(), mThemes.cend(), [themeType](const auto theme){
        return themeType == theme->getThemeType();
    });
    if (iter != mThemes.cend())
    {
        return *iter;
    }

    mThemes.emplace_back(buildTheme(themeType));
    return mThemes.back();
}

std::shared_ptr<Theme> ResourceThemeLoader::buildTheme(ucf::service::model::ThemeType themeType) const
{
    return std::make_shared<Theme>(themeType);
}
}