#include "CameraDevice.h"

#include <opencv2/opencv.hpp>

#include <ucf/Utilities/OSUtils/OSUtils.h>

#include "MediaServiceLogger.h"
#include "VideoFrame.h"

namespace ucf::service {

struct CameraDevice::Impl
{
    explicit Impl(media::CameraSource src) : source(std::move(src)) {}

    media::CameraSource source;
    cv::VideoCapture cap;
};

namespace {
void processFrame(cv::Mat& /*frame*/)
{
    // 预留帧处理逻辑（锐化 / 色彩校正等）
}
} // namespace

CameraDevice::CameraDevice(const media::CameraSource& source)
    : mImpl(std::make_unique<Impl>(source))
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
        SERVICE_LOG_DEBUG("camera already opened, source: " << media::toKey(mImpl->source));
        return true;
    }

    bool result = std::visit([this](auto&& s) -> bool {
        using T = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<T, media::LocalCameraSource>)
        {
            switch (ucf::utilities::OSUtils::getOSType())
            {
            case ucf::utilities::OSType::WINDOWS:
                return mImpl->cap.open(s.index, cv::VideoCaptureAPIs::CAP_DSHOW);
            case ucf::utilities::OSType::LINUX:
                return mImpl->cap.open(s.index, cv::VideoCaptureAPIs::CAP_V4L2);
            default:
                return mImpl->cap.open(s.index);
            }
        }
        else if constexpr (std::is_same_v<T, media::NetworkCameraSource>)
        {
            bool ok = mImpl->cap.open(s.url, cv::CAP_FFMPEG);
            if (ok && mImpl->cap.isOpened())
            {
                mImpl->cap.set(cv::CAP_PROP_BUFFERSIZE, 1);
                if (s.openTimeoutMs > 0)
                {
                    mImpl->cap.set(cv::CAP_PROP_OPEN_TIMEOUT_MSEC, s.openTimeoutMs);

                }
                if (s.readTimeoutMs > 0)
                {
                    mImpl->cap.set(cv::CAP_PROP_READ_TIMEOUT_MSEC, s.readTimeoutMs);
                }
            }
            return ok;
        }
    }, mImpl->source);

    if (result && isOpened())
    {
        SERVICE_LOG_DEBUG("camera opened, source: " << media::toKey(mImpl->source)
            << ", width: " << getFrameWidth()
            << ", height: " << getFrameHeight());
    }
    else
    {
        SERVICE_LOG_WARN("failed to open camera, source: " << media::toKey(mImpl->source));
    }

    return result && isOpened();
}

void CameraDevice::close()
{
    if (isOpened())
    {
        mImpl->cap.release();
        SERVICE_LOG_DEBUG("camera closed, source: " << media::toKey(mImpl->source));
    }
}

bool CameraDevice::isOpened() const
{
    return mImpl->cap.isOpened();
}

media::IVideoFramePtr CameraDevice::readFrame()
{
    if (!isOpened())
    {
        return nullptr;
    }

    cv::Mat frame;
    mImpl->cap.read(frame);
    if (frame.empty())
    {
        return nullptr;
    }

    processFrame(frame);

    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
    std::vector<uint8_t> pixels(rgb.datastart, rgb.dataend);
    return std::make_shared<media::VideoFrame>(
        std::move(pixels), rgb.cols, rgb.rows,
        static_cast<int>(rgb.step), media::PixelFormat::RGB888);
}

const media::CameraSource& CameraDevice::getSource() const
{
    return mImpl->source;
}

int CameraDevice::getFrameWidth() const
{
    return static_cast<int>(mImpl->cap.get(cv::CAP_PROP_FRAME_WIDTH));
}

int CameraDevice::getFrameHeight() const
{
    return static_cast<int>(mImpl->cap.get(cv::CAP_PROP_FRAME_HEIGHT));
}
}
