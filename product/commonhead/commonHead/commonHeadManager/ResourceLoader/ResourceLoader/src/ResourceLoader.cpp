#include "ResourceLoader.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ClientInfoService/ClientInfoModel.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/ResourceStringLoader/IResourceStringLoader.h>
#include <commonHead/ResourceColorLoader/IResourceColorLoader.h>


#include "ResourceThemeLoader.h"

namespace commonHead{
std::shared_ptr<IResourceLoader> IResourceLoader::createInstance(ucf::framework::ICoreFrameworkWPtr coreframework)
{
    return std::make_shared<ResourceLoader>(coreframework);
}

ResourceLoader::ResourceLoader(ucf::framework::ICoreFrameworkWPtr coreframework)
    : mCoreframeworkWPtr(coreframework)
    , mResourceThemeLoader(std::make_unique<ResourceThemeLoader>())
    , mResourceColorLoader(IResourceColorLoader::createInstance())
{
    COMMONHEAD_LOG_DEBUG("create ResourceLoader, this:" << this);
}

void ResourceLoader::initResourceLoader()
{
    COMMONHEAD_LOG_DEBUG("");
    if (auto coreFramework = mCoreframeworkWPtr.lock())
    {
        if (auto service = coreFramework->getService<ucf::service::IClientInfoService>().lock())
        {
            service->registerCallback(shared_from_this());
        }
    }
}

model::Font ResourceLoader::getFont(model::FontFamily family, model::FontSize size, model::FontWeight weight, bool isItalic) const
{
    if (mResourceThemeLoader)
    {
        return mResourceThemeLoader->getFont(getCurrentThemeType(), family, size, weight, isItalic);
    }
    
    COMMONHEAD_LOG_WARN("no mResourceThemeLoader");
    return model::Font();
}

model::Color ResourceLoader::getColor(model::ColorToken colorToken, model::ColorState state) const
{
    return mResourceColorLoader->getColor(colorToken, state, getCurrentColorThemeType());
}

model::ColorThemeType ResourceLoader::getCurrentColorThemeType() const
{
    switch (getCurrentThemeType())
    {
    case ucf::service::model::ThemeType::Light:
        return model::ColorThemeType::Light;
    case ucf::service::model::ThemeType::Dark:
        return model::ColorThemeType::Dark;
    default:
        return model::ColorThemeType::Light;
    }
}

ucf::service::model::ThemeType ResourceLoader::getCurrentThemeType() const
{
    if (auto coreFramework = mCoreframeworkWPtr.lock())
    {
        if (auto service = coreFramework->getService<ucf::service::IClientInfoService>().lock())
        {
            return service->getCurrentThemeType();
        }
    }
    COMMONHEAD_LOG_WARN("no clientInfoService, use default themeType: " << static_cast<int>(ucf::service::model::ThemeType::SystemDefault));
    return ucf::service::model::ThemeType::SystemDefault;
}

void ResourceLoader::setLocalizedStringLoader(std::unique_ptr<IResourceStringLoader>&& resourceStringLoader)
{
    COMMONHEAD_LOG_INFO("set resourceStringLoader");
    mResourceStringLoader = std::move(resourceStringLoader);
}

std::string ResourceLoader::getLocalizedString(model::LocalizedString stringId) const
{
    if (mResourceStringLoader)
    {
        return mResourceStringLoader->getLocalizedString(stringId);
    }
    COMMONHEAD_LOG_WARN("no resourceStringLoader");
    return {};
}

std::string ResourceLoader::getLocalizedStringWithParams(model::LocalizedStringWithParam stringId, const std::initializer_list<std::string>& params) const
{
    if (mResourceStringLoader)
    {
        return mResourceStringLoader->getLocalizedStringWithParams(stringId, params);
    }
    COMMONHEAD_LOG_WARN("no resourceStringLoader");
    return {};
}

void ResourceLoader::onClientThemeChanged(ucf::service::model::ThemeType themeType)
{
    addResourceTheme(themeType);
}

void ResourceLoader::onClientInfoReady()
{
    addResourceTheme(getCurrentThemeType());
}

void ResourceLoader::addResourceTheme(ucf::service::model::ThemeType themeType)
{
    if (mResourceThemeLoader)
    {
        mResourceThemeLoader->addTheme(themeType);
    }
    else
    {
        COMMONHEAD_LOG_WARN("no resourceStringLoader");
    }
}
}