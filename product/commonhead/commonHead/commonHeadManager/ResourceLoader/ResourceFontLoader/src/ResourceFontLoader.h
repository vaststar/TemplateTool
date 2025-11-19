#pragma once

#include <memory>
#include <commonHead/ResourceFontLoader/IResourceFontLoader.h>
#include "ThemeFontSet.h"

namespace commonHead{
class ResourceFontLoader final: public IResourceFontLoader
{
public:
    ResourceFontLoader();
    ResourceFontLoader(const ResourceFontLoader&) = delete;
    ResourceFontLoader(ResourceFontLoader&&) = delete;
    ResourceFontLoader& operator=(const ResourceFontLoader&) = delete;
    ResourceFontLoader& operator=(ResourceFontLoader&&) = delete;
    ~ResourceFontLoader() = default;
public:
    virtual model::Font getFont(model::FontToken fontToken, model::FontThemeType theme) const override;
private:
    void buildThemeFontSets();
private:
    std::map<model::FontThemeType, model::ThemeFontSet> mThemeFontSets;
};
}