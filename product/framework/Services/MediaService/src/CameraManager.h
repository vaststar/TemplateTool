#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <optional>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <ucf/Services/ImageService/ImageTypes.h>

namespace ucf::service::model{
    struct Image;
}

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
    std::optional<model::Image> readImageData(const std::string& cameraId);
private:
    mutable std::mutex mCamerasMutex;
    std::vector<std::unique_ptr<CameraVideoCapture>> mCamerasList;
};
}