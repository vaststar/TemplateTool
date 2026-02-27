#pragma once

#include <string>
#include <memory>
#include <atomic>

#include <ucf/Services/MediaService/MediaTypes.h>

namespace cv {
class Mat;
}

namespace ucf::service {
class CameraDevice;

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
    // 设备状态
    bool isOpened() const;
    std::string getCameraId() const;
    int getCameraNum() const;
    
    // 引用计数
    void addDeviceRef();
    void releaseDeviceRef();
    int getDeviceRefCount() const;
    
    // 帧读取
    media::IVideoFramePtr readImageData();

private:
    void processFrame(cv::Mat& frame) const;
    media::IVideoFramePtr convertFrameToVideoFrame(const cv::Mat& frame) const;

private:
    std::unique_ptr<CameraDevice> mDevice;
    std::string mCameraId;
    std::atomic<int> mDeviceRefCount{0};
};
}