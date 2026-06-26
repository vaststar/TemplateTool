#pragma once

#include <trompeloeil.hpp>
#include <ucf/Services/FeatureSettingsService/IFeatureSettingsService.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

namespace ucf::service::fakes {

/**
 * @brief Fake implementation of IFeatureSettingsService for unit testing
 *
 * Usage:
 *   auto fakeService = std::make_shared<FakeFeatureSettingsService>();
 *   ALLOW_CALL(*fakeService, getScreenshotSettings()).RETURN(model::ScreenshotFeatureSettings{});
 */
class FakeFeatureSettingsService
    : public IFeatureSettingsService
    , public ucf::utilities::NotificationHelper<IFeatureSettingsServiceCallback>
{
public:
    FakeFeatureSettingsService() = default;
    ~FakeFeatureSettingsService() override = default;

    // IService
    MAKE_CONST_MOCK0(getServiceName, std::string(), override);

    // IFeatureSettingsService
    MAKE_CONST_MOCK0(getScreenshotSettings, model::ScreenshotFeatureSettings(), override);
    MAKE_MOCK1(updateScreenshotSettings, void(const model::ScreenshotFeatureSettings&), override);
    MAKE_CONST_MOCK0(getRecordingSettings, model::RecordingFeatureSettings(), override);
    MAKE_MOCK1(updateRecordingSettings, void(const model::RecordingFeatureSettings&), override);

protected:
    void initService() override {}
};

} // namespace ucf::service::fakes
