#include "ResourceLoader.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ClientInfoService/ClientInfoModel.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/ResourceLoader/IResourceStringLoader.h>

#include "ResourceThemeLoader.h"

namespace commonHead{
std::shared_ptr<IResourceLoader> IResourceLoader::createInstance(ucf::framework::ICoreFrameworkWPtr coreframework)
{
    return std::make_shared<ResourceLoader>(coreframework);
}

ResourceLoader::ResourceLoader(ucf::framework::ICoreFrameworkWPtr coreframework)
    : mCoreframeworkWPtr(coreframework)
    , mResourceThemeLoader(std::make_unique<ResourceThemeLoader>())
{

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

model::Color ResourceLoader::getColor(model::ColorItem colorItem, model::ColorItemState state) const
{
    if (mResourceThemeLoader)
    {
        return mResourceThemeLoader->getColor(getCurrentThemeType(), colorItem, state);
    }

    COMMONHEAD_LOG_WARN("no mResourceThemeLoader");
    return model::Color();
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

void ResourceLoader::setResourceLocalizedString(std::unique_ptr<IResourceStringLoader>&& resourceStringLoader)
{
    COMMONHEAD_LOG_WARN("set resourceStringLoader");
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

}