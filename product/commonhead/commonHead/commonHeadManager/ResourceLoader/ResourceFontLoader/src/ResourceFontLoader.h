#pragma once

#include <memory>
#include <commonHead/ResourceFontLoader/IResourceFontLoader.h>
#include "ThemeFontSet.h"

namespace commonHead{
class ResourceFontLoader: public IResourceFontLoader
{
public:
    ResourceFontLoader();
    ~ResourceFontLoader() = default;

    virtual model::Font getFont(model::FontToken fontToken, model::FontThemeType theme) const override;
private:
    void buildThemeFontSets();
private:
    std::map<model::FontThemeType, model::ThemeFontSet> mThemeFontSets;
};
}