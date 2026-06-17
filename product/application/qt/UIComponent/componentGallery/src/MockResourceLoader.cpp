#include "MockResourceLoader.h"

MockResourceLoader::MockResourceLoader()
    : mColorLoader(commonHead::IResourceColorLoader::createInstance())
    , mFontLoader(commonHead::IResourceFontLoader::createInstance())
    , mAssetLoader(commonHead::IResourceAssetLoader::createInstance())
{
}

void MockResourceLoader::toggleTheme()
{
    mTheme = (mTheme == commonHead::model::ColorThemeType::Light)
                 ? commonHead::model::ColorThemeType::Dark
                 : commonHead::model::ColorThemeType::Light;
}

commonHead::model::Color
MockResourceLoader::getColor(commonHead::model::ColorToken token,
                             commonHead::model::ColorState state) const
{
    return mColorLoader->getColor(token, state, mTheme);
}

commonHead::model::Font
MockResourceLoader::getFont(commonHead::model::FontToken token) const
{
    return mFontLoader->getFont(token, commonHead::model::FontThemeType::Normal);
}

std::string
MockResourceLoader::getAssetImageTokenName(commonHead::model::AssetImageToken token) const
{
    return mAssetLoader->getAssetImageTokenName(token, commonHead::model::AssetThemeType::Normal);
}

std::string
MockResourceLoader::getAssetVideoTokenName(commonHead::model::AssetVideoToken token) const
{
    return mAssetLoader->getAssetVideoTokenName(token, commonHead::model::AssetThemeType::Normal);
}
