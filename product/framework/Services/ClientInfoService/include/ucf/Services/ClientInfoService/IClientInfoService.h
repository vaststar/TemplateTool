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

namespace model{
    class SqliteDBConfig;
}
class SERVICE_EXPORT IClientInfoService: public IService, public virtual ucf::utilities::INotificationHelper<IClientInfoServiceCallback>
{
public:
    IClientInfoService() = default;
    IClientInfoService(const IClientInfoService&) = delete;
    IClientInfoService(IClientInfoService&&) = delete;
    IClientInfoService& operator=(const IClientInfoService&) = delete;
    IClientInfoService& operator=(IClientInfoService&&) = delete;
    virtual ~IClientInfoService() = default;
public:
    [[nodiscard]] virtual model::Version getApplicationVersion() const = 0;

    [[nodiscard]] virtual model::ProductInfo getProductInfo() const = 0;

    [[nodiscard]] virtual model::LanguageType getApplicationLanguage() const = 0;
    virtual void setApplicationLanguage(model::LanguageType languageType) = 0;
    [[nodiscard]] virtual std::vector<model::LanguageType> getSupportedLanguages() const = 0;

    virtual void setCurrentThemeType(model::ThemeType themeType) = 0;
    [[nodiscard]] virtual model::ThemeType getCurrentThemeType() const = 0;
    [[nodiscard]] virtual std::vector<model::ThemeType> getSupportedThemeTypes() const = 0;

    [[nodiscard]] virtual model::SqliteDBConfig getSharedDBConfig() const = 0;

    [[nodiscard]] virtual std::string getAppDataStoragePath() const = 0;
    [[nodiscard]] virtual std::string getAppLogStoragePath() const = 0;
    [[nodiscard]] virtual std::string getAppCrashStoragePath() const = 0;
    static std::shared_ptr<IClientInfoService> createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}