#pragma once

#include <atomic>
#include <cstddef>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <ucf/services/MediaService/MediaTypes.h>
#include <ucf/services/MediaService/IMediaService.h>

#include "VideoFrameSubscriptionRegistry.h"

namespace ucf::service {
class CameraDevice;

class CameraVideoCapture
{
public:
    explicit CameraVideoCapture(const media::CameraSource& source);
    CameraVideoCapture(const CameraVideoCapture&) = delete;
    CameraVideoCapture(CameraVideoCapture&&) = delete;
    CameraVideoCapture& operator=(const CameraVideoCapture&) = delete;
    CameraVideoCapture& operator=(CameraVideoCapture&&) = delete;
    ~CameraVideoCapture();

public:
    // 设备状态
    bool isOpened() const;
    std::string getCameraId() const;
    std::string getSourceKey() const;
    const media::CameraSource& getSource() const;

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
    [[nodiscard]] bool ensureCaptureThreadStarted();
    void captureLoop();

private:
    std::unique_ptr<CameraDevice> mDevice;
    std::string mCameraId;
    std::atomic<int> mDeviceRefCount{0};

    VideoFrameSubscriptionRegistry mSubscriptions;

    std::thread mCaptureThread;
    // Serializes the one-time start of mCaptureThread. The thread stays alive
    // and waits while there are no active subscriptions.
    std::mutex mCaptureThreadMutex;

    // All fields below are guarded by mCaptureStateMutex. The subscription
    // count is scheduling state for the condition-variable predicate; the
    // registry remains the source of callback data.
    std::mutex mCaptureStateMutex;
    std::condition_variable mCaptureStateChanged;
    std::size_t mActiveSubscriptionCount{0};
    bool mStopping{false};
};
}
