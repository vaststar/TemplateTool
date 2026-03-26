#pragma once

#include <commonHead/ResourceColorLoader/IResourceColorLoader.h>
#include "ThemeColorSet.h"

namespace commonHead {
class ResourceColorLoader : public IResourceColorLoader
{
public:
    ResourceColorLoader();
    ResourceColorLoader(const ResourceColorLoader&) = delete;
    ResourceColorLoader(ResourceColorLoader&&) = delete;
    ResourceColorLoader& operator=(const ResourceColorLoader&) = delete;
    ResourceColorLoader& operator=(ResourceColorLoader&&) = delete;
    ~ResourceColorLoader() = default;
public:
    virtual model::Color getColor(model::ColorToken token, model::ColorState state, model::ColorThemeType theme) const override;
private:
    void buildThemeColorSets();
private:
    std::map<model::ColorThemeType, model::ThemeColorSet> mThemeColorSets;
};
} // namespace commonHead