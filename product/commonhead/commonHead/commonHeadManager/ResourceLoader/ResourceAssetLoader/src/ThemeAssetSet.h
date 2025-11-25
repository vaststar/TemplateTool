#pragma once

#include <map>
#include <commonHead/ResourceAssetLoader/model/Asset.h>
#include <AssetToken.h>

namespace commonHead::model{

struct AssetImageSet
{
    AssetImageToken token;
    std::string imagePath;
    std::string format;
    bool isAnimated{ false };
};

struct AssetVideoSet
{
    AssetVideoToken token;
    std::string videoPath;
    std::string format;
};

struct ThemeAssetSet
{
    AssetThemeType themeType;
    std::map<AssetImageToken, AssetImageSet> imageSets;
    std::map<AssetVideoToken, AssetVideoSet> videoSets;
};
}