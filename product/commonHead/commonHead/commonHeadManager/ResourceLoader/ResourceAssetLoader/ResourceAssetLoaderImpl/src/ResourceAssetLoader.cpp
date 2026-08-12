#include "ResourceAssetLoader.h"

#include <BuildNormalThemeAssetSet.h>
#include <commonHead/ResourceAssetLoader/ResourceAssetLoaderCreator.h>

#include "LoggerDefine.h"

namespace commonHead {

namespace impl {

std::unique_ptr<IResourceAssetLoader> createResourceAssetLoader()
{
    return std::make_unique<ResourceAssetLoader>();
}

} // namespace impl

ResourceAssetLoader::ResourceAssetLoader()
{
    RESOURCE_ASSET_LOADER_LOG_DEBUG("ResourceAssetLoader created");
    buildThemeAssetSets();
}

std::string ResourceAssetLoader::getAssetImageTokenName(
    model::AssetImageToken assetImageToken,
    model::AssetThemeType theme) const
{
    if (auto it = mThemeAssetSets.find(theme); it != mThemeAssetSets.end())
    {
        const auto& themeAssetSet = it->second;
        if (auto imageSetIt = themeAssetSet.imageSets.find(assetImageToken); imageSetIt != themeAssetSet.imageSets.end())
        {
            return imageSetIt->second.tokenName;
        }
        RESOURCE_ASSET_LOADER_LOG_WARN(
            "cant find asset image, token: " << static_cast<int>(assetImageToken)
            << ", theme: " << static_cast<int>(theme));
    }
    else
    {
        RESOURCE_ASSET_LOADER_LOG_WARN("cant find theme asset set, theme: " << static_cast<int>(theme));
    }
    return {};
}

std::string ResourceAssetLoader::getAssetVideoTokenName(
    model::AssetVideoToken assetVideoToken,
    model::AssetThemeType theme) const
{
    if (auto it = mThemeAssetSets.find(theme); it != mThemeAssetSets.end())
    {
        const auto& themeAssetSet = it->second;
        if (auto videoSetIt = themeAssetSet.videoSets.find(assetVideoToken); videoSetIt != themeAssetSet.videoSets.end())
        {
            return videoSetIt->second.tokenName;
        }
        RESOURCE_ASSET_LOADER_LOG_WARN(
            "cant find asset video, token: " << static_cast<int>(assetVideoToken)
            << ", theme: " << static_cast<int>(theme));
    }
    else
    {
        RESOURCE_ASSET_LOADER_LOG_WARN("cant find theme asset set, theme: " << static_cast<int>(theme));
    }
    return {};
}

void ResourceAssetLoader::buildThemeAssetSets()
{
    mThemeAssetSets[model::AssetThemeType::Normal] = model::buildNormalThemeAssetSet();
}

} // namespace commonHead
