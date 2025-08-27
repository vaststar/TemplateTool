#pragma once

#include <string>
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

#include <commonHead/ResourceFontLoader/ResourceFontModel.h>

namespace commonHead{
class COMMONHEAD_EXPORT IResourceFontLoader
{
public:
    virtual ~IResourceFontLoader() = default;

    virtual model::Font getFont(model::FontFamily family, model::FontSize size, model::FontWeight weight, bool isItalic) const = 0;

    static std::unique_ptr<IResourceFontLoader> createInstance();
};
}