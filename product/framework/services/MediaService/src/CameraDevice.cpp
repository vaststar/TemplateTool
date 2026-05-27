#include "CameraDevice.h"

#include <ucf/Utilities/OSUtils/OSUtils.h>

#include "MediaServiceLogger.h"

namespace ucf::service {
CameraDevice::CameraDevice(const media::CameraSource& source)
    : mSource(source)
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
        SERVICE_LOG_DEBUG("camera already opened, source: " << media::toKey(mSource));
        return true;
    }

    bool result = std::visit([this](auto&& s) -> bool {
        using T = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<T, media::LocalCameraSource>)
        {
            switch (ucf::utilities::OSUtils::getOSType())
            {
            case ucf::utilities::OSType::WINDOWS:
                return mVideoCap.open(s.index, cv::VideoCaptureAPIs::CAP_DSHOW);
            case ucf::utilities::OSType::LINUX:
                return mVideoCap.open(s.index, cv::VideoCaptureAPIs::CAP_V4L2);
            default:
                return mVideoCap.open(s.index);
            }
        }
        else if constexpr (std::is_same_v<T, media::NetworkCameraSource>)
        {
            bool ok = mVideoCap.open(s.url, cv::CAP_FFMPEG);
            if (ok && mVideoCap.isOpened())
            {
                mVideoCap.set(cv::CAP_PROP_BUFFERSIZE, 1);
                if (s.openTimeoutMs > 0)
                {
                    mVideoCap.set(cv::CAP_PROP_OPEN_TIMEOUT_MSEC, s.openTimeoutMs);

                }
                if (s.readTimeoutMs > 0)
                {
                    mVideoCap.set(cv::CAP_PROP_READ_TIMEOUT_MSEC, s.readTimeoutMs);
                }
            }
            return ok;
        }
    }, mSource);

    if (result && isOpened())
    {
        SERVICE_LOG_DEBUG("camera opened, source: " << media::toKey(mSource)
            << ", width: " << getFrameWidth()
            << ", height: " << getFrameHeight());
    }
    else
    {
        SERVICE_LOG_WARN("failed to open camera, source: " << media::toKey(mSource));
    }

    return result && isOpened();
}

void CameraDevice::close()
{
    if (isOpened())
    {
        mVideoCap.release();
        SERVICE_LOG_DEBUG("camera closed, source: " << media::toKey(mSource));
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

const media::CameraSource& CameraDevice::getSource() const
{
    return mSource;
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
