#pragma once

#include <memory>
#include "FontSet.h"

namespace ucf::service::model{
enum class ThemeType;
}
namespace commonHead{

class ResourceTheme
{
public:
    ResourceTheme(ucf::service::model::ThemeType themeType);
    ucf::service::model::ThemeType getThemeType() const;
    model::Font getFont(model::FontFamily family, model::FontSize size, model::FontWeight weight, bool isItalic) const;
private:
    void initFonts();
private:
    ucf::service::model::ThemeType mThemeType;
    std::unique_ptr<FontSet> mFontSet;
};
}