#pragma once

#include <map>
#include <string>

#include <AssetToken.h>
#include <commonHead/ResourceAssetLoader/Asset.h>

namespace commonHead::model {

struct AssetImageSet
{
    AssetImageToken token;
    std::string tokenName;
    std::string imagePath;
    std::string format;
    bool isAnimated{ false };
    std::string groupName;
    std::string fileName;
};

struct AssetVideoSet
{
    AssetVideoToken token;
    std::string tokenName;
    std::string videoPath;
    std::string format;
    std::string groupName;
    std::string fileName;
};

struct ThemeAssetSet
{
    AssetThemeType themeType;
    std::map<AssetImageToken, AssetImageSet> imageSets;
    std::map<AssetVideoToken, AssetVideoSet> videoSets;
};

} // namespace commonHead::model
