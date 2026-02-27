#pragma once

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

namespace ucf::service {
class CameraDevice
{
public:
    explicit CameraDevice(int cameraNum);
    CameraDevice(const CameraDevice&) = delete;
    CameraDevice(CameraDevice&&) = delete;
    CameraDevice& operator=(const CameraDevice&) = delete;
    CameraDevice& operator=(CameraDevice&&) = delete;
    ~CameraDevice();

public:
    bool open();
    void close();
    bool isOpened() const;
    cv::Mat readFrame();
    
    int getCameraNum() const;
    int getFrameWidth() const;
    int getFrameHeight() const;

private:
    const int mCameraNum;
    cv::VideoCapture mVideoCap;
};
}
