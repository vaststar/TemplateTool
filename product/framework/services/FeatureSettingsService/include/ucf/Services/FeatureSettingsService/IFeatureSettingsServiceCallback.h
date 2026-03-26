#pragma once

#include <ucf/Services/FeatureSettingsService/FeatureSettingsModel.h>

namespace ucf::service {

class IFeatureSettingsServiceCallback
{
public:
    IFeatureSettingsServiceCallback() = default;
    IFeatureSettingsServiceCallback(const IFeatureSettingsServiceCallback&) = delete;
    IFeatureSettingsServiceCallback(IFeatureSettingsServiceCallback&&) = delete;
    IFeatureSettingsServiceCallback& operator=(const IFeatureSettingsServiceCallback&) = delete;
    IFeatureSettingsServiceCallback& operator=(IFeatureSettingsServiceCallback&&) = delete;
    virtual ~IFeatureSettingsServiceCallback() = default;
public:
    virtual void onFeatureSettingsReady() = 0;
    virtual void onScreenshotSettingsChanged(const model::ScreenshotFeatureSettings& screenshotSettings) = 0;
    virtual void onRecordingSettingsChanged(const model::RecordingFeatureSettings& recordingSettings) = 0;
};

} // namespace ucf::service
