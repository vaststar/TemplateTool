#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <optional>

#include <ucf/Services/MediaService/MediaTypes.h>

namespace ucf::service{

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
private:
    mutable std::mutex mCamerasMutex;
    std::vector<std::unique_ptr<CameraVideoCapture>> mCamerasList;
};
}