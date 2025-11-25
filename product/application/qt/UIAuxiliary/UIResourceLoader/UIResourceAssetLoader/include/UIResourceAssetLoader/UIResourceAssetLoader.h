#pragma once
#include <UIResourceAssetLoader/UIResourceAssetLoaderExport.h>

namespace commonHead::model{
    enum class AssetImageToken;
    enum class AssetVideoToken;
}

namespace UIAssetImageToken{
    enum class AssetImageToken;
}

namespace UIAssetVideoToken{
    enum class AssetVideoToken;
}

namespace UIResource{
class UIResourceAssetLoader_EXPORT UIResourceAssetLoader final
{
public:
    UIResourceAssetLoader() = delete;
    UIResourceAssetLoader(const UIResourceAssetLoader&) = delete;
    UIResourceAssetLoader(UIResourceAssetLoader&&) = delete;
    UIResourceAssetLoader& operator=(const UIResourceAssetLoader&) = delete;
    UIResourceAssetLoader& operator=(UIResourceAssetLoader&&) = delete;
    ~UIResourceAssetLoader() = delete;
public:
    static void registerUIResourceAssetLoader();
    static commonHead::model::AssetImageToken convertUIAssetImageTokenToVMAssetImageToken(UIAssetImageToken::AssetImageToken  uiAssetImageItem);
    static commonHead::model::AssetVideoToken convertUIAssetVideoTokenToVMAssetVideoToken(UIAssetVideoToken::AssetVideoToken  uiAssetVideoItem);
};
}