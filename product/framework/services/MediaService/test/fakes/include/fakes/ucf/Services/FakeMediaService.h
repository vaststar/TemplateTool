#pragma once

#include <trompeloeil.hpp>
#include <ucf/Services/MediaService/IMediaService.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

namespace ucf::service::fakes {

/**
 * @brief Fake implementation of IMediaService for unit testing
 *
 * Usage:
 *   auto fakeService = std::make_shared<FakeMediaService>();
 *   ALLOW_CALL(*fakeService, getOpenedCameras()).RETURN(std::vector<std::string>{});
 */
class FakeMediaService
    : public IMediaService
    , public ucf::utilities::NotificationHelper<IMediaServiceCallback>
{
public:
    FakeMediaService() = default;
    ~FakeMediaService() override = default;

    // IService
    MAKE_CONST_MOCK0(getServiceName, std::string(), override);

    // IMediaService
    MAKE_MOCK1(openCamera, std::string(const media::CameraSource&), override);
    MAKE_MOCK1(releaseCamera, void(const std::string&), override);
    MAKE_CONST_MOCK0(getOpenedCameras, std::vector<std::string>(), override);
    MAKE_MOCK1(readImageData, media::IVideoFramePtr(const std::string&), override);
    MAKE_MOCK2(startVideoCapture, std::string(const std::string&, VideoFrameCallback), override);
    MAKE_MOCK2(stopVideoCapture, void(const std::string&, const std::string&), override);

protected:
    void initService() override {}
};

} // namespace ucf::service::fakes
