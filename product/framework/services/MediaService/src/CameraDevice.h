#pragma once

#include <memory>

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
    // 读取一帧，返回中立的 RGB888 帧；设备未开或空帧时返回 nullptr
    media::IVideoFramePtr readFrame();

    int getFrameWidth() const;
    int getFrameHeight() const;
    const media::CameraSource& getSource() const;

private:
    struct Impl;                    // 隐藏 cv::VideoCapture 和 source
    std::unique_ptr<Impl> mImpl;
};
}
