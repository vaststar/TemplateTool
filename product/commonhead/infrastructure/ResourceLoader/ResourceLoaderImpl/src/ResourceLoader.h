#pragma once

#include <memory>

#include <commonhead/ResourceLoader/IResourceLoader.h>
#include <ucf/services/ClientInfoService/IClientInfoServiceCallback.h>

namespace ucf::framework {
class ICoreFramework;
}

namespace ucf::service::model {
enum class ThemeType;
}

namespace commonHead {

class IResourceAssetLoader;
class IResourceColorLoader;
class IResourceFontLoader;

class ResourceLoader final : public IResourceLoader,
                             public ucf::service::IClientInfoServiceCallback,
                             public std::enable_shared_from_this<ResourceLoader>
{
public:
    explicit ResourceLoader(std::weak_ptr<ucf::framework::ICoreFramework> coreFramework);
    ~ResourceLoader() override;

    void initResourceLoader() override;
    [[nodiscard]] model::Font getFont(model::FontToken fontToken) const override;
    [[nodiscard]] model::Color getColor(
        model::ColorToken colorToken,
        model::ColorState state) const override;

    void setLocalizedStringLoader(std::unique_ptr<IResourceStringLoader>&& resourceStringLoader) override;
    [[nodiscard]] std::string getNonLocalizedString(model::NonLocalizedString stringId) const override;
    [[nodiscard]] std::string getLocalizedString(model::LocalizedString stringId) const override;
    [[nodiscard]] std::string getLocalizedStringWithParams(
        model::LocalizedStringWithParam stringId,
        const std::initializer_list<std::string>& params) const override;

    [[nodiscard]] std::string getAssetImageTokenName(model::AssetImageToken assetImageToken) const override;
    [[nodiscard]] std::string getAssetVideoTokenName(model::AssetVideoToken assetVideoToken) const override;

private:
    void onClientInfoReady() override;
    void onClientThemeChanged(ucf::service::model::ThemeType themeType) override;
    void addResourceTheme(ucf::service::model::ThemeType themeType);

    [[nodiscard]] ucf::service::model::ThemeType getCurrentThemeType() const;
    [[nodiscard]] model::ColorThemeType getCurrentColorThemeType() const;
    [[nodiscard]] model::FontThemeType getCurrentFontThemeType() const;
    [[nodiscard]] model::AssetThemeType getCurrentAssetThemeType() const;

    const std::weak_ptr<ucf::framework::ICoreFramework> mCoreFramework;
    const std::unique_ptr<IResourceColorLoader> mResourceColorLoader;
    const std::unique_ptr<IResourceFontLoader> mResourceFontLoader;
    std::unique_ptr<IResourceStringLoader> mResourceStringLoader;
    const std::unique_ptr<IResourceAssetLoader> mResourceAssetLoader;
};

} // namespace commonHead
