#pragma once

#include <initializer_list>
#include <memory>
#include <string>

#include <AssetToken.h>
#include <ColorToken.h>
#include <FontToken.h>
#include <ResourceString.h>
#include <commonHead/ResourceAssetLoader/Asset.h>
#include <commonHead/ResourceColorLoader/Color.h>
#include <commonHead/ResourceFontLoader/Font.h>

namespace commonHead {

class IResourceStringLoader;

class IResourceLoader
{
public:
    IResourceLoader() = default;
    IResourceLoader(const IResourceLoader&) = delete;
    IResourceLoader(IResourceLoader&&) = delete;
    IResourceLoader& operator=(const IResourceLoader&) = delete;
    IResourceLoader& operator=(IResourceLoader&&) = delete;
    virtual ~IResourceLoader() = default;

    virtual void initResourceLoader() = 0;
    [[nodiscard]] virtual model::Font getFont(model::FontToken fontToken) const = 0;
    [[nodiscard]] virtual model::Color getColor(
        model::ColorToken colorToken,
        model::ColorState state) const = 0;

    [[nodiscard]] virtual std::string getAssetImageTokenName(model::AssetImageToken assetImageToken) const = 0;
    [[nodiscard]] virtual std::string getAssetVideoTokenName(model::AssetVideoToken assetVideoToken) const = 0;

    // Set by the upper layer before localized strings are requested.
    virtual void setLocalizedStringLoader(std::unique_ptr<IResourceStringLoader>&& resourceStringLoader) = 0;
    [[nodiscard]] virtual std::string getNonLocalizedString(model::NonLocalizedString stringId) const = 0;
    [[nodiscard]] virtual std::string getLocalizedString(model::LocalizedString stringId) const = 0;
    [[nodiscard]] virtual std::string getLocalizedStringWithParams(
        model::LocalizedStringWithParam stringId,
        const std::initializer_list<std::string>& params) const = 0;
};

} // namespace commonHead
