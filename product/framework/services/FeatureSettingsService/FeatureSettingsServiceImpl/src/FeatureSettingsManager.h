#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include <ucf/Services/FeatureSettingsService/FeatureSettingsModel.h>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class FeatureSettingsManager final
{
public:
    explicit FeatureSettingsManager(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~FeatureSettingsManager();
    FeatureSettingsManager(const FeatureSettingsManager&) = delete;
    FeatureSettingsManager(FeatureSettingsManager&&) = delete;
    FeatureSettingsManager& operator=(const FeatureSettingsManager&) = delete;
    FeatureSettingsManager& operator=(FeatureSettingsManager&&) = delete;

public:
    model::ScreenshotFeatureSettings getScreenshotSettings() const;
    void updateScreenshotSettings(const model::ScreenshotFeatureSettings& screenshotSettings);

    model::RecordingFeatureSettings getRecordingSettings() const;
    void updateRecordingSettings(const model::RecordingFeatureSettings& recordingSettings);

    void databaseInitialized(const std::string& databaseId);

private:
    void loadScreenshotSettingsFromDatabase();
    void saveScreenshotSettingsToDatabase();
    void loadRecordingSettingsFromDatabase();
    void saveRecordingSettingsToDatabase();

private:
    const ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    model::ScreenshotFeatureSettings mScreenshotSettings;
    model::RecordingFeatureSettings mRecordingSettings;
    mutable std::mutex mMutex;
};

} // namespace ucf::service
