#pragma once
#include <UIResourceColorLoader/UIResourceColorLoaderExport.h>

namespace commonHead::model{
    enum class ColorState;
    enum class ColorToken;
}

namespace UIColorState{
    enum class ColorState;
}

namespace UIColorToken{
    enum class ColorToken;
}
namespace UIResource{
class UIResourceColorLoader_EXPORT UIResourceColorLoader final
{
public:
    UIResourceColorLoader() = delete;
    UIResourceColorLoader(const UIResourceColorLoader&) = delete;
    UIResourceColorLoader(UIResourceColorLoader&&) = delete;
    UIResourceColorLoader& operator=(const UIResourceColorLoader&) = delete;
    UIResourceColorLoader& operator=(UIResourceColorLoader&&) = delete;
    ~UIResourceColorLoader() = delete;
public:
    static void registerUIResourceColorLoader();
    static commonHead::model::ColorState convertUIColorStateToVMColorState(UIColorState::ColorState uiColorState);
    static commonHead::model::ColorToken convertUIColorTokenToVMColorToken(UIColorToken::ColorToken  uiColorItem);
};
}