#pragma once

#include <string>

#include <AssetToken.h>
#include <commonHead/ResourceAssetLoader/Asset.h>

namespace commonHead {

class IResourceAssetLoader
{
public:
    IResourceAssetLoader() = default;
    IResourceAssetLoader(const IResourceAssetLoader&) = delete;
    IResourceAssetLoader(IResourceAssetLoader&&) = delete;
    IResourceAssetLoader& operator=(const IResourceAssetLoader&) = delete;
    IResourceAssetLoader& operator=(IResourceAssetLoader&&) = delete;
    virtual ~IResourceAssetLoader() = default;

    [[nodiscard]] virtual std::string getAssetImageTokenName(
        model::AssetImageToken assetImageToken,
        model::AssetThemeType theme) const = 0;
    [[nodiscard]] virtual std::string getAssetVideoTokenName(
        model::AssetVideoToken assetVideoToken,
        model::AssetThemeType theme) const = 0;
};

} // namespace commonHead
