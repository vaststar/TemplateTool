#pragma once

#include <FontToken.h>
#include <commonhead/ResourceFontLoader/Font.h>

namespace commonHead {

class IResourceFontLoader
{
public:
    IResourceFontLoader() = default;
    IResourceFontLoader(const IResourceFontLoader&) = delete;
    IResourceFontLoader(IResourceFontLoader&&) = delete;
    IResourceFontLoader& operator=(const IResourceFontLoader&) = delete;
    IResourceFontLoader& operator=(IResourceFontLoader&&) = delete;
    virtual ~IResourceFontLoader() = default;

    [[nodiscard]] virtual model::Font getFont(
        model::FontToken fontToken,
        model::FontThemeType theme) const = 0;
};

} // namespace commonHead
