#pragma once

#include <memory>
#include <string>
#include <initializer_list>

#include <commonHead/ResourceLoader/ResourceLoaderModel.h>
#include <commonHead/ResourceLoader/ResourceLocalizedStringModel.h>
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace commonHead{
class IResourceStringLoader;
class COMMONHEAD_EXPORT IResourceLoader
{
public:
    ~IResourceLoader() = default;
    virtual void initResourceLoader() = 0;
    [[nodiscard]] virtual model::Font getFont(model::FontFamily family, model::FontSize size, model::FontWeight weight, bool isItalic) const = 0;
    [[nodiscard]] virtual model::Color getColor(model::ColorItem colorItem, model::ColorItemState state) const = 0;

    //set string loader from upper layer before you want to use localized string, otherwise it will return empty string
    virtual void setLocalizedStringLoader(std::unique_ptr<IResourceStringLoader>&& resourceStringLoader) = 0;
    [[nodiscard]] virtual std::string getLocalizedString(model::LocalizedString stringId) const = 0;
    [[nodiscard]] virtual std::string getLocalizedStringWithParams(model::LocalizedStringWithParam stringId, const std::initializer_list<std::string>& params) const = 0;
public:
    static std::shared_ptr<IResourceLoader> createInstance(ucf::framework::ICoreFrameworkWPtr coreframework);
};
}