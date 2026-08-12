#pragma once

#include <map>

#include <commonhead/ResourceColorLoader/IResourceColorLoader.h>

#include "ThemeColorSet.h"

namespace commonHead {

class ResourceColorLoader final : public IResourceColorLoader
{
public:
    ResourceColorLoader();

    [[nodiscard]] model::Color getColor(
        model::ColorToken token,
        model::ColorState state,
        model::ColorThemeType theme) const override;

private:
    void buildThemeColorSets();

    std::map<model::ColorThemeType, model::ThemeColorSet> mThemeColorSets;
};

} // namespace commonHead
