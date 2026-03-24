#pragma once

#include <memory>
#include <vector>
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
    void databaseInitialized(const std::string& databaseId);

private:
    void loadScreenshotSettingsFromDatabase();
    void saveScreenshotSettingsToDatabase();

private:
    const ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    model::ScreenshotFeatureSettings mScreenshotSettings;
    mutable std::mutex mMutex;
};

} // namespace ucf::service
