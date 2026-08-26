#pragma once

#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <functional>

#include <ucf/services/MediaService/MediaTypes.h>
#include <ucf/services/MediaService/IMediaService.h>

namespace ucf::service {

class CameraVideoCapture;

class CameraManager
{
public:
    CameraManager();
    CameraManager(const CameraManager&) = delete;
    CameraManager(CameraManager&&) = delete;
    CameraManager& operator=(const CameraManager&) = delete;
    CameraManager& operator=(CameraManager&&) = delete;
    ~CameraManager();

public:
    std::string openCamera(const media::CameraSource& source);
    void releaseCamera(const std::string& cameraId);
    std::vector<std::string> getOpenedCameras() const;
    media::IVideoFramePtr readImageData(const std::string& cameraId);

    // 视频捕获订阅
    std::string startVideoCapture(const std::string& cameraId, VideoFrameCallback callback);
    void stopVideoCapture(const std::string& cameraId, const std::string& subscriptionId);

private:
    mutable std::mutex mCamerasMutex;
    // Operations copy a strong reference while holding mCamerasMutex and use
    // it after unlocking, so releaseCamera() cannot invalidate an in-flight
    // operation.
    std::vector<std::shared_ptr<CameraVideoCapture>> mCamerasList;
};
}
