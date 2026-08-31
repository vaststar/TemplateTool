#pragma once

#include <map>

#include <commonhead/ResourceFontLoader/IResourceFontLoader.h>

#include "ThemeFontSet.h"

namespace commonHead {

class ResourceFontLoader final : public IResourceFontLoader
{
public:
    ResourceFontLoader();
    ~ResourceFontLoader() override;

    [[nodiscard]] model::Font getFont(
        model::FontToken fontToken,
        model::FontThemeType theme) const override;

private:
    void buildThemeFontSets();

    std::map<model::FontThemeType, model::ThemeFontSet> mThemeFontSets;
};

} // namespace commonHead
