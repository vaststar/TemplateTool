#include "ResourceLoader.h"

#include <utility>

#include <commonhead/ResourceAssetLoader/IResourceAssetLoader.h>
#include <commonhead/ResourceAssetLoader/ResourceAssetLoaderCreator.h>
#include <commonhead/ResourceColorLoader/IResourceColorLoader.h>
#include <commonhead/ResourceColorLoader/ResourceColorLoaderCreator.h>
#include <commonhead/ResourceFontLoader/IResourceFontLoader.h>
#include <commonhead/ResourceFontLoader/ResourceFontLoaderCreator.h>
#include <commonhead/ResourceLoader/ResourceLoaderCreator.h>
#include <commonhead/ResourceStringLoader/IResourceStringLoader.h>
#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/services/ClientInfoService/ClientInfoTypes.h>
#include <ucf/services/ClientInfoService/IClientInfoService.h>
#include <ucf/utilities/SystemUtils/SystemUtils.h>

#include "LoggerDefine.h"

namespace commonHead {

namespace impl {

std::shared_ptr<IResourceLoader> createResourceLoader(
    std::weak_ptr<ucf::framework::ICoreFramework> coreFramework)
{
    return std::make_shared<ResourceLoader>(std::move(coreFramework));
}

} // namespace impl

ResourceLoader::ResourceLoader(std::weak_ptr<ucf::framework::ICoreFramework> coreFramework)
    : mCoreFramework(std::move(coreFramework))
    , mResourceColorLoader(impl::createResourceColorLoader())
    , mResourceFontLoader(impl::createResourceFontLoader())
    , mResourceAssetLoader(impl::createResourceAssetLoader())
{
    RESOURCE_LOADER_LOG_DEBUG("create ResourceLoader, this:" << this);
}

ResourceLoader::~ResourceLoader() = default;

void ResourceLoader::initResourceLoader()
{
    RESOURCE_LOADER_LOG_DEBUG("");
    if (auto coreFramework = mCoreFramework.lock())
    {
        if (auto service = coreFramework->getService<ucf::service::IClientInfoService>().lock())
        {
            service->registerCallback(shared_from_this());
        }
    }
}

model::Font ResourceLoader::getFont(model::FontToken fontToken) const
{
    if (mResourceFontLoader)
    {
        return mResourceFontLoader->getFont(fontToken, getCurrentFontThemeType());
    }
    RESOURCE_LOADER_LOG_WARN("no mResourceFontLoader");
    return {};
}

model::Color ResourceLoader::getColor(
    model::ColorToken colorToken,
    model::ColorState state) const
{
    if (mResourceColorLoader)
    {
        return mResourceColorLoader->getColor(colorToken, state, getCurrentColorThemeType());
    }
    RESOURCE_LOADER_LOG_WARN("no mResourceColorLoader");
    return {};
}

model::ColorThemeType ResourceLoader::getCurrentColorThemeType() const
{
    switch (getCurrentThemeType())
    {
        case ucf::service::model::ThemeType::Light:
            return model::ColorThemeType::Light;
        case ucf::service::model::ThemeType::Dark:
            return model::ColorThemeType::Dark;
        case ucf::service::model::ThemeType::SystemDefault:
            return (ucf::utilities::SystemUtils::getSystemColorScheme() == ucf::utilities::SystemColorScheme::Dark)
                ? model::ColorThemeType::Dark
                : model::ColorThemeType::Light;
        default:
            return model::ColorThemeType::Light;
    }
}

model::FontThemeType ResourceLoader::getCurrentFontThemeType() const
{
    switch (getCurrentThemeType())
    {
        case ucf::service::model::ThemeType::Light:
        case ucf::service::model::ThemeType::Dark:
        default:
            return model::FontThemeType::Normal;
    }
}

model::AssetThemeType ResourceLoader::getCurrentAssetThemeType() const
{
    switch (getCurrentThemeType())
    {
        case ucf::service::model::ThemeType::Light:
        case ucf::service::model::ThemeType::Dark:
        default:
            return model::AssetThemeType::Normal;
    }
}

ucf::service::model::ThemeType ResourceLoader::getCurrentThemeType() const
{
    if (auto coreFramework = mCoreFramework.lock())
    {
        if (auto service = coreFramework->getService<ucf::service::IClientInfoService>().lock())
        {
            return service->getCurrentThemeType();
        }
    }
    RESOURCE_LOADER_LOG_WARN(
        "no clientInfoService, use default themeType: "
        << static_cast<int>(ucf::service::model::ThemeType::SystemDefault));
    return ucf::service::model::ThemeType::SystemDefault;
}

void ResourceLoader::setLocalizedStringLoader(std::unique_ptr<IResourceStringLoader>&& resourceStringLoader)
{
    RESOURCE_LOADER_LOG_INFO("set resourceStringLoader");
    mResourceStringLoader = std::move(resourceStringLoader);
}

std::string ResourceLoader::getNonLocalizedString(model::NonLocalizedString stringId) const
{
    if (mResourceStringLoader)
    {
        return mResourceStringLoader->getNonLocalizedString(stringId);
    }
    RESOURCE_LOADER_LOG_WARN("no resourceStringLoader");
    return {};
}

std::string ResourceLoader::getLocalizedString(model::LocalizedString stringId) const
{
    if (mResourceStringLoader)
    {
        return mResourceStringLoader->getLocalizedString(stringId);
    }
    RESOURCE_LOADER_LOG_WARN("no resourceStringLoader");
    return {};
}

std::string ResourceLoader::getLocalizedStringWithParams(
    model::LocalizedStringWithParam stringId,
    const std::initializer_list<std::string>& params) const
{
    if (mResourceStringLoader)
    {
        return mResourceStringLoader->getLocalizedStringWithParams(stringId, params);
    }
    RESOURCE_LOADER_LOG_WARN("no resourceStringLoader");
    return {};
}

std::string ResourceLoader::getAssetImageTokenName(model::AssetImageToken assetImageToken) const
{
    if (mResourceAssetLoader)
    {
        return mResourceAssetLoader->getAssetImageTokenName(assetImageToken, getCurrentAssetThemeType());
    }
    return {};
}

std::string ResourceLoader::getAssetVideoTokenName(model::AssetVideoToken assetVideoToken) const
{
    if (mResourceAssetLoader)
    {
        return mResourceAssetLoader->getAssetVideoTokenName(assetVideoToken, getCurrentAssetThemeType());
    }
    return {};
}

void ResourceLoader::onClientThemeChanged(ucf::service::model::ThemeType themeType)
{
    addResourceTheme(themeType);
}

void ResourceLoader::onClientInfoReady()
{
    addResourceTheme(getCurrentThemeType());
}

void ResourceLoader::addResourceTheme(ucf::service::model::ThemeType themeType)
{
    (void)themeType;
}

} // namespace commonHead
