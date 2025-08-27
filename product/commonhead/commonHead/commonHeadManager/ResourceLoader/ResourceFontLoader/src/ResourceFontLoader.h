#pragma once

#include <memory>
#include <commonHead/ResourceFontLoader/IResourceFontLoader.h>
#include "FontSet.h"

namespace commonHead{
class ResourceFontLoader: public IResourceFontLoader
{
public:
    ResourceFontLoader();
    ~ResourceFontLoader() = default;
    virtual model::Font getFont(model::FontFamily family, model::FontSize size, model::FontWeight weight, bool isItalic) const override;
private:
    void initFonts();
private:
    std::unique_ptr<FontSet> mFontSet;
};
}