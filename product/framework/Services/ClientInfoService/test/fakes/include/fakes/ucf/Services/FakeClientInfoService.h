#pragma once

#include <trompeloeil.hpp>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Services/DataWarehouseService/DatabaseConfig.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

namespace ucf::service::fakes {

/**
 * @brief Fake implementation of IClientInfoService for unit testing
 * 
 * Usage:
 *   auto fakeService = std::make_shared<FakeClientInfoService>();
 *   ALLOW_CALL(*fakeService, getAppCrashStoragePath()).RETURN("/tmp/crash");
 */
class FakeClientInfoService 
    : public IClientInfoService
    , public ucf::utilities::NotificationHelper<IClientInfoServiceCallback>
{
public:
    FakeClientInfoService() = default;
    ~FakeClientInfoService() override = default;

    // IService
    MAKE_CONST_MOCK0(getServiceName, std::string(), override);

    // IClientInfoService - Version & Product
    MAKE_CONST_MOCK0(getApplicationVersion, model::Version(), override);
    MAKE_CONST_MOCK0(getProductInfo, model::ProductInfo(), override);

    // IClientInfoService - Language
    MAKE_CONST_MOCK0(getApplicationLanguage, model::LanguageType(), override);
    MAKE_MOCK1(setApplicationLanguage, void(model::LanguageType), override);
    MAKE_CONST_MOCK0(getSupportedLanguages, std::vector<model::LanguageType>(), override);

    // IClientInfoService - Theme
    MAKE_MOCK1(setCurrentThemeType, void(model::ThemeType), override);
    MAKE_CONST_MOCK0(getCurrentThemeType, model::ThemeType(), override);
    MAKE_CONST_MOCK0(getSupportedThemeTypes, std::vector<model::ThemeType>(), override);

    // IClientInfoService - Database
    MAKE_CONST_MOCK0(getSharedDBConfig, model::SqliteDBConfig(), override);

    // IClientInfoService - Paths
    MAKE_CONST_MOCK0(getAppDataStoragePath, std::string(), override);
    MAKE_CONST_MOCK0(getAppLogStoragePath, std::string(), override);
    MAKE_CONST_MOCK0(getAppCrashStoragePath, std::string(), override);
    MAKE_CONST_MOCK0(getAppHangStoragePath, std::string(), override);

protected:
    void initService() override {}
};

} // namespace ucf::service::fakes
