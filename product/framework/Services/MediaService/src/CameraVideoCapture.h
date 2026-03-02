#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include <functional>
#include <mutex>
#include <vector>

#include <ucf/Services/MediaService/MediaTypes.h>
#include <ucf/Services/MediaService/IMediaService.h>

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
    
    // 设备引用计数
    void addDeviceRef();
    void releaseDeviceRef();
    int getDeviceRefCount() const;
    
    // 帧读取 (单帧，用于截图等)
    media::IVideoFramePtr readImageData();
    
    // 视频捕获订阅
    std::string addSubscription(VideoFrameCallback callback);
    void removeSubscription(const std::string& subscriptionId);

private:
    void processFrame(cv::Mat& frame) const;
    media::IVideoFramePtr convertFrameToVideoFrame(const cv::Mat& frame) const;
    void captureLoop();

private:
    std::unique_ptr<CameraDevice> mDevice;
    std::string mCameraId;
    std::atomic<int> mDeviceRefCount{0};
    
    // 订阅管理
    struct Subscription
    {
        std::string id;
        VideoFrameCallback callback;
    };
    std::vector<Subscription> mSubscriptions;
    std::mutex mSubscriptionMutex;
    
    // 捕获线程
    std::thread mCaptureThread;
    std::atomic<bool> mCapturing{false};
};
}