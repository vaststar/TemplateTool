#pragma once

#include <vector>
#include <memory>

#include <commonHead/ResourceLoader/ResourceLoaderModel.h>
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace commonHead{
class COMMONHEAD_EXPORT IResourceLoader
{
public:
    ~IResourceLoader() = default;
    virtual model::Font getFont(model::FontFamily family, model::FontSize size, model::FontWeight weight, bool isItalic) const = 0;
    virtual model::Color getColor(model::ColorItem colorItem, model::ColorItemState state) const = 0;
public:
    static std::shared_ptr<IResourceLoader> createInstance(ucf::framework::ICoreFrameworkWPtr coreframework);
};
}