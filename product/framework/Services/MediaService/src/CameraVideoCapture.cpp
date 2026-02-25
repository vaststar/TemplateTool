#include "CameraVideoCapture.h"

#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>
#include <ucf/Utilities/OSUtils/OSUtils.h>

#include "MediaServiceLogger.h"

namespace ucf::service{
CameraVideoCapture::CameraVideoCapture(int cameraNum)
    : mCameraNum(cameraNum)
{
    //mVideoCap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    //mVideoCap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    if (openCamera() && isOpened())
    {
        mRefCount = 1;
        mCameraId = ucf::utilities::UUIDUtils::generateUUID();
        
        //mVideoCap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
        //mVideoCap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
        double width = mVideoCap.get(cv::CAP_PROP_FRAME_WIDTH);
        double height = mVideoCap.get(cv::CAP_PROP_FRAME_HEIGHT);

        // mVideoCap.set(cv::CAP_PROP_AUTO_WB, 0);
        // mVideoCap.set(cv::CAP_PROP_WB_TEMPERATURE, 5000);  // 5000K色温
        // // 自动曝光补偿
        // mVideoCap.set(cv::CAP_PROP_AUTO_EXPOSURE, 0.25);  // 手动曝光模式
        // mVideoCap.set(cv::CAP_PROP_EXPOSURE, -4);         // 根据环境调整
        SERVICE_LOG_DEBUG("camera opened, index: " << cameraNum << ", id: " << mCameraId << ", width: " << width << ", height: " << height);
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

bool CameraVideoCapture::openCamera()
{
    switch (ucf::utilities::OSUtils::getOSType())
    {
    case ucf::utilities::OSType::WINDOWS:
        return mVideoCap.open(mCameraNum, cv::VideoCaptureAPIs::CAP_DSHOW);
    case ucf::utilities::OSType::LINUX:
        return mVideoCap.open(mCameraNum, cv::VideoCaptureAPIs::CAP_V4L2);
    default:
        return mVideoCap.open(mCameraNum);
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
        SERVICE_LOG_DEBUG("camera use count decreased, index: " << mCameraNum << ", id: " << mCameraId << ", count: " << mRefCount);
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

std::optional<media::VideoFrame> CameraVideoCapture::readImageData()
{
    if (!isOpened())
    {
        SERVICE_LOG_WARN("camera not opened, index: " << mCameraNum << ", id: " << mCameraId);
        return std::nullopt;
    }

    if (cv::Mat frame; mVideoCap.read(frame) && !frame.empty())
    {
        processFrame(frame);
        return convertFrameToVideoFrame(frame);
    }
    else
    {
        SERVICE_LOG_WARN("read empty frame:" << mCameraId);
        return std::nullopt;
    }
    return std::nullopt;
}

void CameraVideoCapture::processFrame(cv::Mat& frame) const
{
    // cv::Mat blurred, detail;
    // cv::GaussianBlur(frame, blurred, cv::Size(0,0), 5);
    // cv::subtract(frame, blurred, detail);
    // cv::addWeighted(frame, 1.2, detail, 0.5, 0, frame);


    // cv::Mat labImg, claheImg;
    // cv::cvtColor(frame, labImg, cv::COLOR_BGR2Lab);
    // std::vector<cv::Mat> channels;
    // cv::split(labImg, channels);
    
    // cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8,8));
    // clahe->apply(channels[0], claheImg);
    // channels[0] = claheImg;
    // cv::merge(channels, labImg);
    // cv::cvtColor(labImg, frame, cv::COLOR_Lab2BGR);

//     cv::Mat blurred, sharpened;
// cv::GaussianBlur(frame, blurred, cv::Size(0,0), 3);
// cv::addWeighted(frame, 1.5, blurred, -0.5, 0, sharpened);
}

media::VideoFrame CameraVideoCapture::convertFrameToVideoFrame(const cv::Mat& frame) const
{
    cv::Mat rgbFrame;
    cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);
    std::vector<uchar> vec(rgbFrame.datastart, rgbFrame.dataend);
    return media::VideoFrame(std::move(vec), rgbFrame.cols, rgbFrame.rows, 
                             static_cast<int>(rgbFrame.step), media::PixelFormat::RGB888);
}
}
