#pragma once

#include <string>
#include <vector>

#include <commonHead/ResourceFontLoader/ResourceFontLoaderTypesExport.h>

namespace commonHead::model {

struct RESOURCE_FONT_LOADER_TYPES_API Font
{
    std::vector<std::string> fontFamilies;
    int fontSize{ 12 };
    int fontWeight{ 400 };
    bool isItalic{ false };
};

enum class FontThemeType
{
    Normal,
    Large
};

} // namespace commonHead::model
