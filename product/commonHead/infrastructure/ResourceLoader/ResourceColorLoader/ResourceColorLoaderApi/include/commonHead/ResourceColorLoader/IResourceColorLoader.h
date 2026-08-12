#pragma once

#include <ColorToken.h>
#include <commonHead/ResourceColorLoader/Color.h>

namespace commonHead {

class IResourceColorLoader
{
public:
    IResourceColorLoader() = default;
    IResourceColorLoader(const IResourceColorLoader&) = delete;
    IResourceColorLoader(IResourceColorLoader&&) = delete;
    IResourceColorLoader& operator=(const IResourceColorLoader&) = delete;
    IResourceColorLoader& operator=(IResourceColorLoader&&) = delete;
    virtual ~IResourceColorLoader() = default;

    [[nodiscard]] virtual model::Color getColor(
        model::ColorToken token,
        model::ColorState state,
        model::ColorThemeType theme) const = 0;
};

} // namespace commonHead
