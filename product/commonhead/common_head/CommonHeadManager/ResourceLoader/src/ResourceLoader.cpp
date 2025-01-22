#include "ResourceLoader.h"

#include <thread>

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ClientInfoService/ClientInfoModel.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/ResourceLoader/IResourceStringLoader.h>


namespace commonHead{
std::shared_ptr<IResourceLoader> IResourceLoader::createInstance(ucf::framework::ICoreFrameworkWPtr coreframework)
{
    return std::make_shared<ResourceLoader>(coreframework);
}

ResourceLoader::ResourceLoader(ucf::framework::ICoreFrameworkWPtr coreframework)
    : mCoreframeworkWPtr(coreframework)
{

}

model::Font ResourceLoader::getFont(model::FontFamily family, model::FontSize size, model::FontWeight weight, bool isItalic) const
{
    if (auto theme = getOrCreateTheme(getCurrentThemeType()))
    {
        return theme->getFont(family, size, weight, isItalic);
    }
    COMMONHEAD_LOG_WARN("can't find theme");
    return model::Font();
}

model::Color ResourceLoader::getColor(model::ColorItem colorItem, model::ColorItemState state) const
{
    if (auto theme = getOrCreateTheme(getCurrentThemeType()))
    {
         return theme->getColor(colorItem, state);
    }
    COMMONHEAD_LOG_WARN("can't find theme");
    return model::Color();
}

ucf::service::model::ThemeType ResourceLoader::getCurrentThemeType() const
{
    if (auto coreFramework = mCoreframeworkWPtr.lock())
    {
        if (auto service = coreFramework->getService<ucf::service::IClientInfoService>().lock())
        {
            return service->getCurrentThemeType();
        }
    }
    COMMONHEAD_LOG_WARN("no clientInfoService, use default themeType: " << static_cast<int>(ucf::service::model::ThemeType::SystemDefault));
    return ucf::service::model::ThemeType::SystemDefault;
}

std::shared_ptr<Theme> ResourceLoader::getOrCreateTheme(ucf::service::model::ThemeType themeType) const
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

std::shared_ptr<Theme> ResourceLoader::buildTheme(ucf::service::model::ThemeType themeType) const
{
    return std::make_shared<Theme>(themeType);
}

void ResourceLoader::setResourceLocalizedString(std::unique_ptr<IResourceStringLoader>&& resourceStringLoader)
{
    COMMONHEAD_LOG_WARN("set resourceStringLoader");
    mResourceStringLoader = std::move(resourceStringLoader);
}

std::string ResourceLoader::getLocalizedString(model::LocalizedString stringId) const
{
    if (mResourceStringLoader)
    {
        return mResourceStringLoader->getLocalizedString(stringId);
    }
    return {};
}

std::string ResourceLoader::getLocalizedStringWithParams(model::LocalizedStringWithParam stringId, const std::initializer_list<std::string>& params) const
{
    if (mResourceStringLoader)
    {
        return mResourceStringLoader->getLocalizedStringWithParams(stringId, params);
    }
    return {};
}

}