#pragma once

#include <map>

#include <commonHead/ResourceAssetLoader/IResourceAssetLoader.h>

#include "ThemeAssetSet.h"

namespace commonHead {

class ResourceAssetLoader final : public IResourceAssetLoader
{
public:
    ResourceAssetLoader();

    [[nodiscard]] std::string getAssetImageTokenName(
        model::AssetImageToken assetImageToken,
        model::AssetThemeType theme) const override;
    [[nodiscard]] std::string getAssetVideoTokenName(
        model::AssetVideoToken assetVideoToken,
        model::AssetThemeType theme) const override;

private:
    void buildThemeAssetSets();

    std::map<model::AssetThemeType, model::ThemeAssetSet> mThemeAssetSets;
};

} // namespace commonHead
