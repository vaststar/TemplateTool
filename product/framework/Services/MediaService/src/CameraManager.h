#pragma once

#include <string>
#include <map>
#include <mutex>
#include <optional>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <ucf/Services/ImageService/ImageTypes.h>

namespace ucf::service::model{
    struct Image;
}

namespace ucf::service{
class CameraManager
{
public:
    ~CameraManager();
    std::string openCamera(int cameraNum);
    std::vector<std::string> getOpenedCameras() const;
    std::optional<model::Image> readImageData(const std::string& cameraId);
private:
    mutable std::mutex mCamerasMutex;
    std::map<std::string, std::unique_ptr<cv::VideoCapture>> mCameras;
};
}