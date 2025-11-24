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

void ResourceAssetLoader::buildThemeAssetSets()
{
    mThemeAssetSets[model::AssetThemeType::Normal] = model::buildNormalThemeAssetSet();
    // Add other theme Asset sets as needed.
}
} // namespace commonHead