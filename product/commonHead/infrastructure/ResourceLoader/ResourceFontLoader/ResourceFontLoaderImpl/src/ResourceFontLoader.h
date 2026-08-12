#pragma once

#include <map>

#include <commonHead/ResourceFontLoader/IResourceFontLoader.h>

#include "ThemeFontSet.h"

namespace commonHead {

class ResourceFontLoader final : public IResourceFontLoader
{
public:
    ResourceFontLoader();

    [[nodiscard]] model::Font getFont(
        model::FontToken fontToken,
        model::FontThemeType theme) const override;

private:
    void buildThemeFontSets();

    std::map<model::FontThemeType, model::ThemeFontSet> mThemeFontSets;
};

} // namespace commonHead
