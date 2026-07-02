#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <typeindex>

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>
#include <ucf/CoreFramework/CoreFrameworkCallbackDefault.h>
#include <ucf/Services/MiniAppService/IMiniAppService.h>

#include "MiniAppNotificationSink.h"

namespace ucf::service {

class SERVICE_EXPORT MiniAppService final
    : public virtual IMiniAppService
    , public virtual ucf::utilities::NotificationHelper<IMiniAppServiceCallback>
    , public ucf::framework::CoreFrameworkCallbackDefault
    , public IMiniAppNotificationSink
    , public std::enable_shared_from_this<MiniAppService>
{
public:
    explicit MiniAppService(ucf::framework::ICoreFrameworkWPtr coreFramework);
    virtual ~MiniAppService();
    MiniAppService(const MiniAppService&) = delete;
    MiniAppService(MiniAppService&&) = delete;
    MiniAppService& operator=(const MiniAppService&) = delete;
    MiniAppService& operator=(MiniAppService&&) = delete;

public:
    // IService
    virtual std::string getServiceName() const override;
    virtual std::vector<ServiceDependency> dependencies() const override;

    // ICoreFrameworkCallback
    virtual void onServiceInitialized() override;
    virtual void onCoreFrameworkExit() override;

    // IMiniAppService
    virtual bool isReady() const override;
    virtual std::vector<model::MiniAppManifest> listInstalledApps() const override;
    virtual std::optional<model::MiniAppManifest> getApp(const std::string& id) const override;
    virtual bool installFromDirectory(const std::string& sourceDirectory) override;
    virtual bool uninstall(const std::string& id) override;
    virtual std::string getAppPackageDir(const std::string& id) const override;
    virtual std::string getAppStorageDir(const std::string& id) const override;
    virtual std::string getAppCacheDir(const std::string& id) const override;
    virtual std::string getAppIconPath(const std::string& id) const override;

protected:
    // IService
    virtual void initService() override;
    virtual void deinitService() override;

private:
    // IMiniAppNotificationSink — translation layer from internal Manager events
    // to outward IMiniAppServiceCallback notifications.
    virtual void onMiniAppServiceReady() override;
    virtual void onMiniAppInstalled(const model::MiniAppManifest& app) override;
    virtual void onMiniAppUninstalled(const std::string& id) override;

private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};

} // namespace ucf::service
