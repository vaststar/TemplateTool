#pragma once

#include <trompeloeil.hpp>
#include <ucf/Services/MiniAppService/IMiniAppService.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

namespace ucf::service::fakes {

/**
 * @brief Fake implementation of IMiniAppService for unit testing
 *
 * Usage:
 *   auto fakeService = std::make_shared<FakeMiniAppService>();
 *   ALLOW_CALL(*fakeService, listInstalledApps()).RETURN(std::vector<model::MiniAppManifest>{});
 */
class FakeMiniAppService
    : public IMiniAppService
    , public ucf::utilities::NotificationHelper<IMiniAppServiceCallback>
{
public:
    FakeMiniAppService() = default;
    ~FakeMiniAppService() override = default;

    // IService
    MAKE_CONST_MOCK0(getServiceName, std::string(), override);

    // IMiniAppService
    MAKE_CONST_MOCK0(isReady, bool(), override);
    MAKE_CONST_MOCK0(listInstalledApps, std::vector<model::MiniAppManifest>(), override);
    MAKE_CONST_MOCK1(getApp, std::optional<model::MiniAppManifest>(const std::string&), override);
    MAKE_MOCK1(installFromDirectory, bool(const std::string&), override);
    MAKE_MOCK1(uninstall, bool(const std::string&), override);
    MAKE_CONST_MOCK1(getAppPackageDir, std::string(const std::string&), override);
    MAKE_CONST_MOCK1(getAppStorageDir, std::string(const std::string&), override);
    MAKE_CONST_MOCK1(getAppCacheDir, std::string(const std::string&), override);
    MAKE_CONST_MOCK1(getAppIconPath, std::string(const std::string&), override);

protected:
    void initService() override {}
    void deinitService() override {}
};

} // namespace ucf::service::fakes
