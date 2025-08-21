#pragma once

#include <commonHead/ResourceColorLoader/IResourceColorLoader.h>
#include <ThemeColorSet.h>

namespace commonHead {
class ResourceColorLoader : public IResourceColorLoader
{
public:
    ResourceColorLoader();
    ~ResourceColorLoader() = default;
    virtual model::Color getColor(model::ColorToken token, model::ColorState state, model::ColorThemeType theme) const override;
private:
    void buildThemeColorSets();
private:
    std::map<model::ColorThemeType, model::ThemeColorSet> mThemeColorSets;
};
} // namespace commonHead