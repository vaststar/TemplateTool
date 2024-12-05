#pragma once

#include <functional>
#include <vector>
#include <memory>
#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>

#include <ucf/Services/ServiceDeclaration/IService.h>
#include <ucf/Services/ClientInfoService/ClientInfoModel.h>
#include <ucf/Services/ClientInfoService/IClientInfoServiceCallback.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{

class SERVICE_EXPORT IClientInfoService: public IService, 
                                      public virtual ucf::utilities::INotificationHelper<IClientInfoServiceCallback>
{
public:
    virtual model::Version getApplicationVersion() const = 0;
    virtual model::LanguageType getApplicationLanguage() const = 0;
    virtual void setApplicationLanguage(model::LanguageType languageType) = 0;
    virtual std::vector<model::LanguageType> getSupportedLanguages() const = 0;
    static std::shared_ptr<IClientInfoService> createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}