#include "CameraVideoCapture.h"

#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>

#include "MediaServiceLogger.h"

namespace ucf::service{
CameraVideoCapture::CameraVideoCapture(int cameraNum)
    : mCameraNum(cameraNum)
{
    if (mVideoCap.open(cameraNum); mVideoCap.isOpened())
    {
        mRefCount = 1;
        mCameraId = ucf::utilities::UUIDUtils::generateUUID();
        SERVICE_LOG_DEBUG("camera opened, index: " << cameraNum << ", id: " << mCameraId);
    }
    else
    {
        SERVICE_LOG_WARN("camera not opened, index: " << cameraNum);
    }
}

CameraVideoCapture::~CameraVideoCapture()
{
    if (mVideoCap.isOpened())
    {
        mVideoCap.release();
        SERVICE_LOG_DEBUG("camera released, index: " << mCameraNum << ", id: " << mCameraId);
    }
    else
    {
        SERVICE_LOG_WARN("camera not opened, won't release, index: " << mCameraNum << ", id: " << mCameraId);
    }
}

int CameraVideoCapture::getCameraNum() const
{
    return mCameraNum;
}

bool CameraVideoCapture::isOpened() const
{
    return mVideoCap.isOpened();
}

std::string CameraVideoCapture::getCameraId() const
{
    return mCameraId;
}

void CameraVideoCapture::addUseCount()
{
    if (!isOpened())
    {
        SERVICE_LOG_WARN("camera not opened, index: " << mCameraNum << ", id: " << mCameraId);
        return;
    }

    if (mRefCount > 0)
    {
        mRefCount++;
        SERVICE_LOG_DEBUG("camera use count increased, index: " << mCameraNum << ", id: " << mCameraId << ", count: " << mRefCount);
    }
    else
    {
        SERVICE_LOG_WARN("camera not opened, index: " << mCameraNum << ", id: " << mCameraId);
    }
}

void CameraVideoCapture::decreaseUseCount()
{
    if (!isOpened())
    {
        SERVICE_LOG_WARN("camera not opened, index: " << mCameraNum << ", id: " << mCameraId);
        return;
    }
    
    if (mRefCount > 0)
    {
        mRefCount--;
        SERVICE_LOG_DEBUG("camera use count increased, index: " << mCameraNum << ", id: " << mCameraId << ", count: " << mRefCount);
    }
    else
    {
        SERVICE_LOG_WARN("camera not opened, index: " << mCameraNum << ", id: " << mCameraId);
    }

    if (mRefCount <= 0)
    {
        mVideoCap.release();
        SERVICE_LOG_DEBUG("camera released, index: " << mCameraNum << ", id: " << mCameraId);
    }
}

int CameraVideoCapture::getUseCount() const
{
    if (!isOpened())
    {
        SERVICE_LOG_WARN("camera not opened, index: " << mCameraNum << ", id: " << mCameraId);
        return 0;
    }

    return mRefCount.load();
}

std::optional<model::Image> CameraVideoCapture::readImageData()
{
    if (!isOpened())
    {
        SERVICE_LOG_WARN("camera not opened, index: " << mCameraNum << ", id: " << mCameraId);
        return std::nullopt;
    }

    if (cv::Mat frame; mVideoCap.read(frame) && !frame.empty())
    {
        return convertFrameToImage(frame);
    }
    else
    {
        SERVICE_LOG_WARN("read empty frame:" << mCameraId);
        return std::nullopt;
    }
    return std::nullopt;
}

model::Image CameraVideoCapture::convertFrameToImage(const cv::Mat& frame) const
{
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    std::vector<uchar> vec(frame.datastart, frame.dataend);
    return model::Image(std::move(vec), frame.cols, frame.rows, frame.step);
}
