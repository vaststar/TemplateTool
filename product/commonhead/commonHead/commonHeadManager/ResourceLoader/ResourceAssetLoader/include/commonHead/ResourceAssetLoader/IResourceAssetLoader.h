#pragma once

#include <memory>
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
// #include <AssetToken.h>

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

    // virtual model::Color getIconPath(model::ColorToken token, model::ColorState state, model::ColorThemeType theme) const = 0;

    static std::unique_ptr<IResourceAssetLoader> createInstance();
};
} // namespace commonHead