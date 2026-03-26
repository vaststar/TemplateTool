#pragma once

#include <memory>
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

#include <commonHead/ResourceAssetLoader/model/Asset.h>
#include <AssetToken.h>

namespace commonHead{
class COMMONHEAD_EXPORT IResourceAssetLoader
{
public:
    IResourceAssetLoader() = default;
    IResourceAssetLoader(const IResourceAssetLoader&) = delete;
    IResourceAssetLoader(IResourceAssetLoader&&) = delete;
    IResourceAssetLoader& operator=(const IResourceAssetLoader&) = delete;
    IResourceAssetLoader& operator=(IResourceAssetLoader&&) = delete;
    virtual ~IResourceAssetLoader() = default;
public:
    [[nodiscard]] virtual std::string getAssetImageTokenName(model::AssetImageToken assetImageToken, model::AssetThemeType theme) const = 0;
    [[nodiscard]] virtual std::string getAssetVideoTokenName(model::AssetVideoToken assetVideoToken, model::AssetThemeType theme) const = 0;

    static std::unique_ptr<IResourceAssetLoader> createInstance();
};
} // namespace commonHead