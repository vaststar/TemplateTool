#pragma once

#include <vector>
#include <memory>

#include <ucf/Services/ClientInfoService/IClientInfoServiceCallback.h>

#include <commonHead/ResourceLoader/IResourceLoader.h>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service::model{
    enum class ThemeType;
}
namespace commonHead{
class IResourceStringLoader;
class ResourceThemeLoader;
class ResourceLoader: public IResourceLoader,
                      public ucf::service::IClientInfoServiceCallback,
                      public std::enable_shared_from_this<ResourceLoader>
{
public:
    explicit ResourceLoader(ucf::framework::ICoreFrameworkWPtr coreframework);
    virtual void initResourceLoader() override;
    virtual model::Font getFont(model::FontFamily family, model::FontSize size, model::FontWeight weight, bool isItalic) const override;
    virtual model::Color getColor(model::ColorItem colorItem, model::ColorItemState state) const override;
    
    virtual void setLocalizedStringLoader(std::unique_ptr<IResourceStringLoader>&& resourceStringLoader) override;
    virtual std::string getLocalizedString(model::LocalizedString stringId) const override;
    virtual std::string getLocalizedStringWithParams(model::LocalizedStringWithParam stringId, const std::initializer_list<std::string>& params) const override;
private:
    //IClientInfoServiceCallback
    virtual void onClientInfoReady() override;
    virtual void onClientThemeChanged(ucf::service::model::ThemeType themeType) override;
private:
    void addResourceTheme(ucf::service::model::ThemeType themeType);
private:
    ucf::service::model::ThemeType getCurrentThemeType() const;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreframeworkWPtr;
    const std::unique_ptr<ResourceThemeLoader> mResourceThemeLoader;
    std::unique_ptr<IResourceStringLoader> mResourceStringLoader;
};
}