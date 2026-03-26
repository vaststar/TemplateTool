#include "CameraDevice.h"

#include <ucf/Utilities/OSUtils/OSUtils.h>

#include "MediaServiceLogger.h"

namespace ucf::service {
CameraDevice::CameraDevice(int cameraNum)
    : mCameraNum(cameraNum)
{
}

CameraDevice::~CameraDevice()
{
    close();
}

bool CameraDevice::open()
{
    if (isOpened())
    {
        SERVICE_LOG_DEBUG("camera already opened, index: " << mCameraNum);
        return true;
    }
    
    bool result = false;
    switch (ucf::utilities::OSUtils::getOSType())
    {
    case ucf::utilities::OSType::WINDOWS:
        result = mVideoCap.open(mCameraNum, cv::VideoCaptureAPIs::CAP_DSHOW);
        break;
    case ucf::utilities::OSType::LINUX:
        result = mVideoCap.open(mCameraNum, cv::VideoCaptureAPIs::CAP_V4L2);
        break;
    default:
        result = mVideoCap.open(mCameraNum);
        break;
    }
    
    if (result && isOpened())
    {
        SERVICE_LOG_DEBUG("camera opened, index: " << mCameraNum 
            << ", width: " << getFrameWidth() 
            << ", height: " << getFrameHeight());
    }
    else
    {
        SERVICE_LOG_WARN("failed to open camera, index: " << mCameraNum);
    }
    
    return result && isOpened();
}

void CameraDevice::close()
{
    if (isOpened())
    {
        mVideoCap.release();
        SERVICE_LOG_DEBUG("camera closed, index: " << mCameraNum);
    }
}

bool CameraDevice::isOpened() const
{
    return mVideoCap.isOpened();
}

cv::Mat CameraDevice::readFrame()
{
    cv::Mat frame;
    if (isOpened())
    {
        mVideoCap.read(frame);
    }
    return frame;
}

int CameraDevice::getCameraNum() const
{
    return mCameraNum;
}

int CameraDevice::getFrameWidth() const
{
    return static_cast<int>(mVideoCap.get(cv::CAP_PROP_FRAME_WIDTH));
}

int CameraDevice::getFrameHeight() const
{
    return static_cast<int>(mVideoCap.get(cv::CAP_PROP_FRAME_HEIGHT));
}
}
