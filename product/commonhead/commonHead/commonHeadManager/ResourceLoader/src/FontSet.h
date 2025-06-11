#pragma once 

#include <string>
#include <vector>
#include <memory>
#include <commonHead/ResourceLoader/ResourceLoaderModel.h>

namespace commonHead{

class Fonts
{
public:
    Fonts(model::FontFamily family);
    model::Font getFont(model::FontSize size, model::FontWeight weight, bool isItalic);
    model::FontFamily getFontFamily();
private:
    model::FontFamily mFontFamly;
    std::string mFontFamilyName;
};

struct FontSet
{
public:
    FontSet();
    void initFonts(const std::vector<std::shared_ptr<Fonts>>& fonts);
    std::shared_ptr<Fonts> getFonts(model::FontFamily family);
private:
    std::vector<std::shared_ptr<Fonts>> mFonts;
};
}