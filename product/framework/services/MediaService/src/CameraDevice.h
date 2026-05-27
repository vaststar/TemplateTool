#pragma once

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include <ucf/Services/MediaService/MediaTypes.h>

namespace ucf::service {
class CameraDevice
{
public:
    explicit CameraDevice(const media::CameraSource& source);
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

    int getFrameWidth() const;
    int getFrameHeight() const;
    const media::CameraSource& getSource() const;

private:
    media::CameraSource mSource;
    cv::VideoCapture mVideoCap;
};
}
