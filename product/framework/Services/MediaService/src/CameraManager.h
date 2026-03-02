#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <functional>

#include <ucf/Services/MediaService/MediaTypes.h>
#include <ucf/Services/MediaService/IMediaService.h>

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
    std::string openCamera(int cameraNum);
    void releaseCamera(const std::string& cameraId);
    std::vector<std::string> getOpenedCameras() const;
    media::IVideoFramePtr readImageData(const std::string& cameraId);
    
    // 视频捕获订阅
    std::string startVideoCapture(const std::string& cameraId, VideoFrameCallback callback);
    void stopVideoCapture(const std::string& cameraId, const std::string& subscriptionId);

private:
    mutable std::mutex mCamerasMutex;
    std::vector<std::unique_ptr<CameraVideoCapture>> mCamerasList;
};
}