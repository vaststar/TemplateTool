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
    static void registerMetaObject();
    static commonHead::model::ColorState convertUIColoStateToVMColorState(UIColorState::ColorState uiColorState);
    static commonHead::model::ColorToken convertUIColorTokenToVMColorToken(UIColorToken::ColorToken  uiColorItem);
};
}