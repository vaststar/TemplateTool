#pragma once

#include <string>
#include <optional>
#include <atomic>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include <ucf/Services/ImageService/ImageTypes.h>

namespace ucf::service{
class CameraVideoCapture
{
public:
    explicit CameraVideoCapture(int cameraNum);
    ~CameraVideoCapture();
public:
    bool isOpened() const;
    std::string getCameraId() const;
    int getCameraNum() const;
    std::optional<model::Image> readImageData();
    void addUseCount();
    void decreaseUseCount();
    int getUseCount() const;
private:
    cv::Mat processFrame(const cv::Mat& frame) const;
    model::Image convertFrameToImage(const cv::Mat& frame) const;
private:
    const int mCameraNum;
    cv::VideoCapture mVideoCap;
    std::string mCameraId;
    std::atomic<int> mRefCount{ 0 };
};
}