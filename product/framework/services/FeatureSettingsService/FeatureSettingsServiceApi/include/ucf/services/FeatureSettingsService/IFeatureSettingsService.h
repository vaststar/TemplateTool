#pragma once

#include <functional>
#include <memory>

#include <ucf/utilities/NotificationHelper/INotificationHelper.h>
#include <ucf/services/ServiceDeclaration/IService.h>
#include <ucf/services/FeatureSettingsService/FeatureSettingsModel.h>
#include <ucf/services/FeatureSettingsService/IFeatureSettingsServiceCallback.h>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class IFeatureSettingsService
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

    [[nodiscard]] virtual model::RecordingFeatureSettings getRecordingSettings() const = 0;
    virtual void updateRecordingSettings(const model::RecordingFeatureSettings& recordingSettings) = 0;
};

} // namespace ucf::service
