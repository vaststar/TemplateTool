#pragma once

#include <commonHead/ResourceAssetLoader/IResourceAssetLoader.h>
#include "ThemeAssetSet.h"

namespace commonHead {
class ResourceAssetLoader : public IResourceAssetLoader
{
public:
    ResourceAssetLoader();
    ResourceAssetLoader(const ResourceAssetLoader&) = delete;
    ResourceAssetLoader(ResourceAssetLoader&&) = delete;
    ResourceAssetLoader& operator=(const ResourceAssetLoader&) = delete;
    ResourceAssetLoader& operator=(ResourceAssetLoader&&) = delete;
    ~ResourceAssetLoader() = default;
public:
    // virtual model::Asset getAsset(model::AssetToken token, model::AssetState state, model::AssetThemeType theme) const override;
private:
    void buildThemeAssetSets();
private:
    std::map<model::AssetThemeType, model::ThemeAssetSet> mThemeAssetSets;
};
} // namespace commonHead