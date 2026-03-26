#include "CameraVideoCapture.h"
#include "CameraDevice.h"
#include "VideoFrame.h"

#include <algorithm>
#include <opencv2/opencv.hpp>

#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>

#include "MediaServiceLogger.h"

namespace ucf::service {
CameraVideoCapture::CameraVideoCapture(int cameraNum)
    : mDevice(std::make_unique<CameraDevice>(cameraNum))
{
    if (mDevice->open())
    {
        mDeviceRefCount = 1;
        mCameraId = ucf::utilities::UUIDUtils::generateUUID();
        SERVICE_LOG_DEBUG("CameraVideoCapture created, cameraNum: " << cameraNum 
            << ", cameraId: " << mCameraId);
    }
    else
    {
        SERVICE_LOG_WARN("CameraVideoCapture created but device not opened, cameraNum: " << cameraNum);
    }
}

CameraVideoCapture::~CameraVideoCapture()
{
    // 停止捕获线程
    {
        std::scoped_lock lock(mSubscriptionMutex);
        mCapturing = false;
    }
    if (mCaptureThread.joinable())
    {
        mCaptureThread.join();
    }
    
    SERVICE_LOG_DEBUG("CameraVideoCapture destroyed, cameraId: " << mCameraId);
}

bool CameraVideoCapture::isOpened() const
{
    return mDevice && mDevice->isOpened();
}

std::string CameraVideoCapture::getCameraId() const
{
    return mCameraId;
}

int CameraVideoCapture::getCameraNum() const
{
    return mDevice ? mDevice->getCameraNum() : -1;
}

void CameraVideoCapture::addDeviceRef()
{
    if (!isOpened())
    {
        SERVICE_LOG_WARN("cannot add ref, device not opened, cameraId: " << mCameraId);
        return;
    }
    
    mDeviceRefCount++;
    SERVICE_LOG_DEBUG("device ref added, cameraId: " << mCameraId 
        << ", refCount: " << mDeviceRefCount);
}

void CameraVideoCapture::releaseDeviceRef()
{
    if (mDeviceRefCount <= 0)
    {
        SERVICE_LOG_WARN("cannot release ref, refCount already 0, cameraId: " << mCameraId);
        return;
    }
    
    mDeviceRefCount--;
    SERVICE_LOG_DEBUG("device ref released, cameraId: " << mCameraId 
        << ", refCount: " << mDeviceRefCount);
    
    if (mDeviceRefCount <= 0 && mDevice)
    {
        mDevice->close();
        SERVICE_LOG_DEBUG("device closed due to refCount 0, cameraId: " << mCameraId);
    }
}

int CameraVideoCapture::getDeviceRefCount() const
{
    return mDeviceRefCount.load();
}

media::IVideoFramePtr CameraVideoCapture::readImageData()
{
    if (!isOpened())
    {
        SERVICE_LOG_WARN("cannot read, device not opened, cameraId: " << mCameraId);
        return nullptr;
    }
    
    cv::Mat frame = mDevice->readFrame();
    if (frame.empty())
    {
        SERVICE_LOG_WARN("read empty frame, cameraId: " << mCameraId);
        return nullptr;
    }
    
    processFrame(frame);
    return convertFrameToVideoFrame(frame);
}

void CameraVideoCapture::processFrame(cv::Mat& frame) const
{
    // 预留帧处理逻辑（如锐化、色彩校正等）
}

media::IVideoFramePtr CameraVideoCapture::convertFrameToVideoFrame(const cv::Mat& frame) const
{
    cv::Mat rgbFrame;
    cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);
    std::vector<uchar> vec(rgbFrame.datastart, rgbFrame.dataend);
    return std::make_shared<media::VideoFrame>(
        std::move(vec), rgbFrame.cols, rgbFrame.rows,
        static_cast<int>(rgbFrame.step), media::PixelFormat::RGB888);
}

std::string CameraVideoCapture::addSubscription(VideoFrameCallback callback)
{
    if (!isOpened() || mDeviceRefCount <= 0)
    {
        SERVICE_LOG_WARN("cannot add subscription, device not available, cameraId: " << mCameraId);
        return {};
    }
    
    std::string subscriptionId = ucf::utilities::UUIDUtils::generateUUID();
    bool shouldStart = false;
    
    {
        std::scoped_lock lock(mSubscriptionMutex);
        shouldStart = mSubscriptions.empty();
        mSubscriptions.push_back({subscriptionId, std::move(callback)});
    }
    
    if (shouldStart)
    {
        mCapturing = true;
        mCaptureThread = std::thread(&CameraVideoCapture::captureLoop, this);
        SERVICE_LOG_DEBUG("capture started, cameraId: " << mCameraId);
    }
    
    SERVICE_LOG_DEBUG("subscription added, cameraId: " << mCameraId 
        << ", subscriptionId: " << subscriptionId);
    return subscriptionId;
}

void CameraVideoCapture::removeSubscription(const std::string& subscriptionId)
{
    bool shouldStop = false;
    
    {
        std::scoped_lock lock(mSubscriptionMutex);
        auto it = std::find_if(mSubscriptions.begin(), mSubscriptions.end(),
            [&subscriptionId](const Subscription& sub) { return sub.id == subscriptionId; });
        
        if (it != mSubscriptions.end())
        {
            mSubscriptions.erase(it);
            SERVICE_LOG_DEBUG("subscription removed, cameraId: " << mCameraId 
                << ", subscriptionId: " << subscriptionId);
        }
        else
        {
            SERVICE_LOG_WARN("subscription not found, cameraId: " << mCameraId 
                << ", subscriptionId: " << subscriptionId);
            return;
        }
        
        if (mSubscriptions.empty())
        {
            mCapturing = false;
            shouldStop = true;
        }
    }
    
    if (shouldStop)
    {
        if (mCaptureThread.joinable())
        {
            mCaptureThread.join();
        }
        SERVICE_LOG_DEBUG("capture stopped, cameraId: " << mCameraId);
    }
}

void CameraVideoCapture::captureLoop()
{
    SERVICE_LOG_DEBUG("capture loop started, cameraId: " << mCameraId);
    constexpr auto targetFrameTime = std::chrono::milliseconds(33);  // ~30fps
    
    while (mCapturing && isOpened() && mDeviceRefCount > 0)
    {
        auto frameStart = std::chrono::steady_clock::now();
        
        if (auto frame = readImageData())
        {
            std::scoped_lock lock(mSubscriptionMutex);
            for (const auto& subscription : mSubscriptions)
            {
                if (subscription.callback)
                {
                    subscription.callback(frame);
                }
            }
        }
        
        auto elapsed = std::chrono::steady_clock::now() - frameStart;
        if (elapsed < targetFrameTime)
        {
            std::this_thread::sleep_for(targetFrameTime - elapsed);
        }
    }
    
    SERVICE_LOG_DEBUG("capture loop stopped, cameraId: " << mCameraId);
}
}
