#pragma once

#include <optional>
#include <vector>
#include <functional>
#include <ucf/Services/ServiceDeclaration/IService.h>
#include <ucf/Services/MediaService/MediaTypes.h>
#include <ucf/Services/MediaService/IMediaServiceCallback.h>
#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

using VideoFrameCallback = std::function<void(const media::IVideoFramePtr&)>;

class SERVICE_EXPORT IMediaService : public IService,
                                     public virtual ucf::utilities::INotificationHelper<IMediaServiceCallback>
{
public:
    IMediaService() = default;
    IMediaService(const IMediaService&) = delete;
    IMediaService(IMediaService&&) = delete;
    IMediaService& operator=(const IMediaService&) = delete;
    IMediaService& operator=(IMediaService&&) = delete;
    virtual ~IMediaService() = default;
    static std::shared_ptr<IMediaService> createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);

public:
    /**
     * * @brief open camera
     * * @param cameraNum camera number, such as 0, 1, 2
     * * @return camera id, if failed, return empty string
     * * @note camera id is a unique string, used to identify the camera
     */
    virtual std::string openCamera(int cameraNum) = 0;

    /**
     * @brief release camera
     * @param cameraId camera id
     * @note you should release camera after use, otherwise, the camera will be occupied and cannot be used by other process
     */
    virtual void releaseCamera(const std::string& cameraId) = 0;
    
    virtual std::vector<std::string> getOpenedCameras() const = 0;
    virtual media::IVideoFramePtr readImageData(const std::string& cameraId) = 0;
    
    /**
     * @brief start video capture and subscribe to frames
     * @param cameraId camera id
     * @param callback callback function to receive frames
     * @return subscription id, used to stop capture later
     */
    virtual std::string startVideoCapture(
        const std::string& cameraId,
        VideoFrameCallback callback) = 0;
    
    /**
     * @brief stop video capture subscription
     * @param cameraId camera id
     * @param subscriptionId subscription id returned from startVideoCapture
     */
    virtual void stopVideoCapture(
        const std::string& cameraId,
        const std::string& subscriptionId) = 0;
};
}