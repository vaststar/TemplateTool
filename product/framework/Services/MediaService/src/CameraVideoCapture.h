#pragma once

#include <string>
#include <optional>
#include <atomic>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include <ucf/Services/MediaService/MediaTypes.h>

namespace ucf::service{
class CameraVideoCapture
{
public:
    explicit CameraVideoCapture(int cameraNum);
    CameraVideoCapture(const CameraVideoCapture&) = delete;
    CameraVideoCapture(CameraVideoCapture&&) = delete;
    CameraVideoCapture& operator=(const CameraVideoCapture&) = delete;
    CameraVideoCapture& operator=(CameraVideoCapture&&) = delete;
    ~CameraVideoCapture();
public:
    bool isOpened() const;
    std::string getCameraId() const;
    int getCameraNum() const;
    media::IVideoFramePtr readImageData();
    void addUseCount();
    void decreaseUseCount();
    int getUseCount() const;
private:
    void processFrame(cv::Mat& frame) const;
    media::IVideoFramePtr convertFrameToVideoFrame(const cv::Mat& frame) const;
    bool openCamera();
private:
    const int mCameraNum;
    cv::VideoCapture mVideoCap;
    std::string mCameraId;
    std::atomic<int> mRefCount{ 0 };
};
}