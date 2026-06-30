#pragma once

#include <string>
#include <vector>
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace commonHead::model{

struct COMMONHEAD_EXPORT Font
{
    std::vector<std::string> fontFamilies;
    int fontSize{ 12 };
    int fontWeight{ 400 };
    bool isItalic{ false };
};

enum class FontThemeType {
    Normal,
    Large
};
}
