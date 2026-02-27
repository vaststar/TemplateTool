#include "CameraVideoCapture.h"
#include "CameraDevice.h"
#include "VideoFrame.h"

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
}
