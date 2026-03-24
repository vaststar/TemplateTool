#pragma once

#include <functional>
#include <memory>

#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>
#include <ucf/Services/ServiceDeclaration/IService.h>
#include <ucf/Services/FeatureSettingsService/FeatureSettingsModel.h>
#include <ucf/Services/FeatureSettingsService/IFeatureSettingsServiceCallback.h>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class SERVICE_EXPORT IFeatureSettingsService
    : public IService
    , public virtual ucf::utilities::INotificationHelper<IFeatureSettingsServiceCallback>
{
public:
    IFeatureSettingsService() = default;
    IFeatureSettingsService(const IFeatureSettingsService&) = delete;
    IFeatureSettingsService(IFeatureSettingsService&&) = delete;
    IFeatureSettingsService& operator=(const IFeatureSettingsService&) = delete;
    IFeatureSettingsService& operator=(IFeatureSettingsService&&) = delete;
    virtual ~IFeatureSettingsService() = default;

public:
    [[nodiscard]] virtual model::ScreenshotFeatureSettings getScreenshotSettings() const = 0;
    virtual void updateScreenshotSettings(const model::ScreenshotFeatureSettings& screenshotSettings) = 0;

public:
    static std::shared_ptr<IFeatureSettingsService> createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};

} // namespace ucf::service
