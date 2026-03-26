#pragma once

#include <string>
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

#include <commonHead/ResourceFontLoader/model/Font.h>
#include <FontToken.h>

namespace commonHead{
class COMMONHEAD_EXPORT IResourceFontLoader
{
public:
    IResourceFontLoader() = default;
    IResourceFontLoader(const IResourceFontLoader&) = delete;
    IResourceFontLoader(IResourceFontLoader&&) = delete;
    IResourceFontLoader& operator=(const IResourceFontLoader&) = delete;
    IResourceFontLoader& operator=(IResourceFontLoader&&) = delete;
    virtual ~IResourceFontLoader() = default;
public:

    virtual model::Font getFont(model::FontToken fontToken, model::FontThemeType theme) const = 0;

    static std::unique_ptr<IResourceFontLoader> createInstance();
};
}