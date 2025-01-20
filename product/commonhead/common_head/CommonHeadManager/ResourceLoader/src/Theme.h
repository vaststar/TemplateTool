#pragma once

#include <memory>
#include "FontSet.h"
#include "ColorSet.h"

namespace ucf::service::model{
enum class ThemeType;
}
namespace commonHead{

class Theme
{
public:
    Theme(ucf::service::model::ThemeType themeType);
    ucf::service::model::ThemeType getThemeType() const;
    model::Font getFont(model::FontFamily family, model::FontSize size, model::FontWeight weight, bool isItalic) const;
    model::Color getColor(model::ColorItem colorItem, model::ColorItemState state) const;
private:
    void initFonts();
    void initColors();
private:
    ucf::service::model::ThemeType mThemeType;
    std::unique_ptr<FontSet> mFontSet;
    std::unique_ptr<ColorSet> mColorSet;
};
}