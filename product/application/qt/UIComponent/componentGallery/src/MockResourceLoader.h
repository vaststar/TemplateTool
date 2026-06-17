#pragma once

#include <memory>
#include <string>

#include <commonHead/ResourceLoader/IResourceLoader.h>
#include <commonHead/ResourceColorLoader/IResourceColorLoader.h>
#include <commonHead/ResourceFontLoader/IResourceFontLoader.h>
#include <commonHead/ResourceAssetLoader/IResourceAssetLoader.h>

/**
 * MockResourceLoader - gallery-only implementation of IResourceLoader.
 *
 * Holds the real color/font/asset loaders (their data is codegen-compiled
 * into the binary, identical to production), but the theme type (Light/Dark)
 * is controlled by a local member, so it can be toggled freely without an
 * ICoreFramework / IClientInfoService.
 */
class MockResourceLoader : public commonHead::IResourceLoader
{
public:
    MockResourceLoader();

    // —— theme control (gallery-specific) ——
    void setThemeType(commonHead::model::ColorThemeType theme) { mTheme = theme; }
    commonHead::model::ColorThemeType themeType() const { return mTheme; }
    void toggleTheme();

    // —— IResourceLoader ——
    void initResourceLoader() override {}
    commonHead::model::Font  getFont(commonHead::model::FontToken fontToken) const override;
    commonHead::model::Color getColor(commonHead::model::ColorToken colorToken,
                                      commonHead::model::ColorState state) const override;

    std::string getAssetImageTokenName(commonHead::model::AssetImageToken assetImageToken) const override;
    std::string getAssetVideoTokenName(commonHead::model::AssetVideoToken assetVideoToken) const override;

    void setLocalizedStringLoader(std::unique_ptr<commonHead::IResourceStringLoader>&&) override {}
    std::string getNonLocalizedString(commonHead::model::NonLocalizedString) const override { return {}; }
    std::string getLocalizedString(commonHead::model::LocalizedString) const override { return {}; }
    std::string getLocalizedStringWithParams(commonHead::model::LocalizedStringWithParam,
                                             const std::initializer_list<std::string>&) const override { return {}; }

private:
    commonHead::model::ColorThemeType mTheme{ commonHead::model::ColorThemeType::Light };
    std::unique_ptr<commonHead::IResourceColorLoader> mColorLoader;
    std::unique_ptr<commonHead::IResourceFontLoader>  mFontLoader;
    std::unique_ptr<commonHead::IResourceAssetLoader> mAssetLoader;
};
