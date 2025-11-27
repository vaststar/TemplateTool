#include "ResourceAssetLoader.h"

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <BuildNormalThemeAssetSet.h>



namespace commonHead {
std::unique_ptr<IResourceAssetLoader> IResourceAssetLoader::createInstance()
{
    return std::make_unique<ResourceAssetLoader>();
}

ResourceAssetLoader::ResourceAssetLoader()
{
    COMMONHEAD_LOG_DEBUG("ResourceAssetLoader created");
    buildThemeAssetSets();
}

std::string ResourceAssetLoader::getAssetImagePath(model::AssetImageToken assetImageToken, model::AssetThemeType theme) const
{
     if (auto it = mThemeAssetSets.find(theme); it != mThemeAssetSets.end())
    {
        const auto& themeAssetSet = it->second;
        if (auto imageSetIt = themeAssetSet.imageSets.find(assetImageToken); imageSetIt != themeAssetSet.imageSets.end())
        {
            const auto& imageSet = imageSetIt->second;
            return imageSet.imagePath;
        }
        else
        {
            COMMONHEAD_LOG_WARN("cant find asset image, token: " << static_cast<int>(assetImageToken) << ", theme: " << static_cast<int>(theme));
        }
    }
    else
    {
        COMMONHEAD_LOG_WARN("cant find theme asset set, theme: " << static_cast<int>(theme));
    }
    return {};
}

std::string ResourceAssetLoader::getAssetVideoPath(model::AssetVideoToken assetVideoToken, model::AssetThemeType theme) const
{
    if (auto it = mThemeAssetSets.find(theme); it != mThemeAssetSets.end())
    {
        const auto& themeAssetSet = it->second;
        if (auto videoSetIt = themeAssetSet.videoSets.find(assetVideoToken); videoSetIt != themeAssetSet.videoSets.end())
        {
            const auto& videoSet = videoSetIt->second;
            return videoSet.videoPath;
        }
        else
        {
            COMMONHEAD_LOG_WARN("cant find asset video, token: " << static_cast<int>(assetVideoToken) << ", theme: " << static_cast<int>(theme));
        }
    }
    else
    {
        COMMONHEAD_LOG_WARN("cant find theme asset set, theme: " << static_cast<int>(theme));
    }
    return {};
}

void ResourceAssetLoader::buildThemeAssetSets()
{
    mThemeAssetSets[model::AssetThemeType::Normal] = model::buildNormalThemeAssetSet();
    // Add other theme Asset sets as needed.
}
} // namespace commonHead