#include "ResourceThemeLoader.h"

#include <ucf/Services/ClientInfoService/ClientInfoModel.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>

#include "FontSet.h"
#include "ColorSet.h"
#include "ResourceTheme.h"

namespace commonHead{

ResourceThemeLoader::ResourceThemeLoader()
{

}

model::Font ResourceThemeLoader::getFont(ucf::service::model::ThemeType themeType, model::FontFamily family, model::FontSize size, model::FontWeight weight, bool isItalic) const
{
    if (auto theme = getTheme(themeType))
    {
        return theme->getFont(family, size, weight, isItalic);
    }
    COMMONHEAD_LOG_WARN("can't find theme");
    return model::Font();
}

model::Color ResourceThemeLoader::getColor(ucf::service::model::ThemeType themeType, model::ColorItem colorItem, model::ColorItemState state) const
{
    if (auto theme = getTheme(themeType))
    {
         return theme->getColor(colorItem, state);
    }
    COMMONHEAD_LOG_WARN("can't find theme");
    return model::Color();
}

std::shared_ptr<ResourceTheme> ResourceThemeLoader::buildTheme(ucf::service::model::ThemeType themeType) const
{
    COMMONHEAD_LOG_INFO("create theme: " << static_cast<int>(themeType));
    return std::make_shared<ResourceTheme>(themeType);
}

void ResourceThemeLoader::addTheme(ucf::service::model::ThemeType themeType)
{
    std::scoped_lock<std::mutex> loc(mThemeMutex);
    auto iter = std::find_if(mThemes.cbegin(), mThemes.cend(), [themeType](const auto theme){
        return themeType == theme->getThemeType();
    });
    if (iter == mThemes.cend())
    {
        mThemes.emplace_back(buildTheme(themeType));
    }
}

std::shared_ptr<ResourceTheme> ResourceThemeLoader::getTheme(ucf::service::model::ThemeType themeType) const
{
    std::scoped_lock<std::mutex> loc(mThemeMutex);
    auto iter = std::find_if(mThemes.cbegin(), mThemes.cend(), [themeType](const auto theme){
        return themeType == theme->getThemeType();
    });
    if (iter != mThemes.cend())
    {
        return *iter;
    }
    return nullptr;
}
}