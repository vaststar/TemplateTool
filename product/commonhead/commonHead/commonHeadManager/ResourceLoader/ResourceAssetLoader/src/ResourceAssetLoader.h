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
    std::string getAssetImageTokenName(model::AssetImageToken assetImageToken, model::AssetThemeType theme) const override;
    std::string getAssetVideoTokenName(model::AssetVideoToken assetVideoToken, model::AssetThemeType theme) const override;
private:
    void buildThemeAssetSets();
private:
    std::map<model::AssetThemeType, model::ThemeAssetSet> mThemeAssetSets;
};
} // namespace commonHead