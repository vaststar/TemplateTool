#pragma once

#include <mutex>
#include <vector>
#include <memory>

#include <commonHead/ResourceLoader/IResourceLoader.h>
#include <commonHead/ResourceLoader/IResourceStringLoader.h>

#include "Theme.h"

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace commonHead{
class ResourceLoader: public IResourceLoader
{
public:
    explicit ResourceLoader(ucf::framework::ICoreFrameworkWPtr coreframework);
    virtual model::Font getFont(model::FontFamily family, model::FontSize size, model::FontWeight weight, bool isItalic) const override;
    virtual model::Color getColor(model::ColorItem colorItem, model::ColorItemState state) const override;
    
    virtual void setResourceLocalizedString(std::unique_ptr<IResourceStringLoader>&& resourceStringLoader) override;
    virtual std::string getLocalizedString(model::LocalizedString stringId) const override;
    virtual std::string getLocalizedStringWithParams(model::LocalizedStringWithParam stringId, const std::initializer_list<std::string>& params) const override;
private:
    ucf::service::model::ThemeType getCurrentThemeType() const;
    std::shared_ptr<Theme> getOrCreateTheme(ucf::service::model::ThemeType themeType) const;
    std::shared_ptr<Theme> buildTheme(ucf::service::model::ThemeType themeType) const;
private:
    ucf::framework::ICoreFrameworkWPtr mCoreframeworkWPtr;
    mutable std::mutex mThemeMutex;
    mutable std::vector<std::shared_ptr<Theme>> mThemes;
    std::once_flag mResourceStringInitFlag;
    std::unique_ptr<IResourceStringLoader> mResourceStringLoader;
};
}