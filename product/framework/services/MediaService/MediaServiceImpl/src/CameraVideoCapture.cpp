#include "CameraVideoCapture.h"
#include "CameraDevice.h"

#include <chrono>
#include <exception>
#include <utility>

#include <ucf/utilities/UUIDUtils/UUIDUtils.h>

#include "MediaServiceLogger.h"

namespace ucf::service {
CameraVideoCapture::CameraVideoCapture(const media::CameraSource& source)
    : mDevice(std::make_unique<CameraDevice>(source))
{
    if (mDevice->open())
    {
        mDeviceRefCount = 1;
        mCameraId = ucf::utilities::UUIDUtils::generateUUID();
        SERVICE_LOG_DEBUG("CameraVideoCapture created, source: " << media::toKey(source)
            << ", cameraId: " << mCameraId);
    }
    else
    {
        SERVICE_LOG_WARN("CameraVideoCapture created but device not opened, source: " << media::toKey(source));
    }
}

CameraVideoCapture::~CameraVideoCapture()
{
    std::size_t activeSubscriptionCount = 0;
    {
        std::scoped_lock lock(mCaptureStateMutex);
        mStopping = true;
        activeSubscriptionCount = mActiveSubscriptionCount;
        mActiveSubscriptionCount = 0;
    }
    mSubscriptions.deactivateAll();
    mCaptureStateChanged.notify_all();

    SERVICE_LOG_DEBUG("stopping capture thread, cameraId: " << mCameraId
        << ", activeSubscriptions: " << activeSubscriptionCount);
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

std::string CameraVideoCapture::getSourceKey() const
{
    return mDevice ? media::toKey(mDevice->getSource()) : std::string{};
}

const media::CameraSource& CameraVideoCapture::getSource() const
{
    static const media::CameraSource kEmpty{media::LocalCameraSource{}};
    return mDevice ? mDevice->getSource() : kEmpty;
}

void CameraVideoCapture::addDeviceRef()
{
    if (!isOpened())
    {
        SERVICE_LOG_WARN("cannot add ref, device not opened, cameraId: " << mCameraId);
        return;
    }

    int refCount = 0;
    {
        std::scoped_lock lock(mCaptureStateMutex);
        refCount = ++mDeviceRefCount;
    }
    mCaptureStateChanged.notify_all();
    SERVICE_LOG_DEBUG("device ref added, cameraId: " << mCameraId
        << ", refCount: " << refCount);
}

void CameraVideoCapture::releaseDeviceRef()
{
    int refCount = 0;
    {
        std::scoped_lock lock(mCaptureStateMutex);
        if (mDeviceRefCount <= 0)
        {
            SERVICE_LOG_WARN("cannot release ref, refCount already 0, cameraId: " << mCameraId);
            return;
        }

        refCount = --mDeviceRefCount;
    }
    mCaptureStateChanged.notify_all();
    SERVICE_LOG_DEBUG("device ref released, cameraId: " << mCameraId
        << ", refCount: " << refCount);

    // CameraManager drops its shared ownership when the count reaches zero.
    // CameraDevice is closed by CameraVideoCapture destruction after all
    // in-flight operations release their local shared_ptr.
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

    auto frame = mDevice->readFrame();
    if (!frame)
    {
        SERVICE_LOG_WARN("read empty frame, cameraId: " << mCameraId);
        return nullptr;
    }
    return frame;
}

std::string CameraVideoCapture::addSubscription(VideoFrameCallback callback)
{
    if (!callback || !isOpened())
    {
        SERVICE_LOG_WARN("cannot add subscription, device not available, cameraId: " << mCameraId);
        return {};
    }

    const auto subscriptionId = mSubscriptions.add(std::move(callback));
    if (subscriptionId.empty())
    {
        SERVICE_LOG_WARN("failed to register camera callback, cameraId: " << mCameraId);
        return {};
    }

    std::size_t activeSubscriptionCount = 0;
    bool accepted = false;
    {
        std::scoped_lock lock(mCaptureStateMutex);
        if (!mStopping && mDeviceRefCount > 0)
        {
            accepted = true;
            activeSubscriptionCount = ++mActiveSubscriptionCount;
        }
    }

    if (!accepted)
    {
        mSubscriptions.remove(subscriptionId);
        SERVICE_LOG_WARN("camera is unavailable; subscription rejected, cameraId: "
            << mCameraId << ", subscriptionId: " << subscriptionId
            << ", deviceRefCount: " << mDeviceRefCount.load());
        return {};
    }

    if (!ensureCaptureThreadStarted())
    {
        mSubscriptions.remove(subscriptionId);
        {
            std::scoped_lock lock(mCaptureStateMutex);
            --mActiveSubscriptionCount;
        }
        mCaptureStateChanged.notify_all();
        SERVICE_LOG_ERROR("subscription rolled back because capture thread could not start, cameraId: "
            << mCameraId << ", subscriptionId: " << subscriptionId);
        return {};
    }

    mCaptureStateChanged.notify_all();

    SERVICE_LOG_DEBUG("subscription added, cameraId: " << mCameraId
        << ", subscriptionId: " << subscriptionId
        << ", activeCount: " << activeSubscriptionCount);
    return subscriptionId;
}

void CameraVideoCapture::removeSubscription(const std::string& subscriptionId)
{
    if (!mSubscriptions.remove(subscriptionId))
    {
        SERVICE_LOG_WARN("subscription not found, cameraId: " << mCameraId
            << ", subscriptionId: " << subscriptionId);
        return;
    }

    std::size_t activeSubscriptionCount = 0;
    {
        std::scoped_lock lock(mCaptureStateMutex);
        if (mActiveSubscriptionCount == 0)
        {
            SERVICE_LOG_ERROR("subscription count underflow, cameraId: " << mCameraId
                << ", subscriptionId: " << subscriptionId);
        }
        else
        {
            --mActiveSubscriptionCount;
        }
        activeSubscriptionCount = mActiveSubscriptionCount;
    }
    mCaptureStateChanged.notify_all();

    SERVICE_LOG_DEBUG("subscription removed, cameraId: " << mCameraId
        << ", subscriptionId: " << subscriptionId
        << ", activeCount: " << activeSubscriptionCount);
}

bool CameraVideoCapture::ensureCaptureThreadStarted()
{
    std::scoped_lock lock(mCaptureThreadMutex);
    if (mCaptureThread.joinable())
    {
        return true;
    }

    try
    {
        mCaptureThread = std::thread(&CameraVideoCapture::captureLoop, this);
        SERVICE_LOG_DEBUG("capture thread started, cameraId: " << mCameraId);
        return true;
    }
    catch (const std::exception& exception)
    {
        SERVICE_LOG_ERROR("failed to start capture thread, cameraId: "
            << mCameraId << ", error: " << exception.what());
        return false;
    }
}

void CameraVideoCapture::captureLoop()
{
    SERVICE_LOG_DEBUG("capture loop started, cameraId: " << mCameraId);
    constexpr auto targetFrameTime = std::chrono::milliseconds(33);  // ~30fps

    while (true)
    {
        bool resumedFromIdle = false;
        {
            std::unique_lock lock(mCaptureStateMutex);
            const bool shouldWait =
                mActiveSubscriptionCount == 0 || mDeviceRefCount <= 0;
            if (shouldWait && !mStopping)
            {
                SERVICE_LOG_DEBUG("capture thread entering idle state, cameraId: " << mCameraId
                    << ", activeSubscriptions: " << mActiveSubscriptionCount
                    << ", deviceRefCount: " << mDeviceRefCount.load());
            }

            mCaptureStateChanged.wait(lock, [this] {
                return mStopping ||
                    (mActiveSubscriptionCount > 0 && mDeviceRefCount > 0);
            });

            if (mStopping)
            {
                break;
            }
            resumedFromIdle = shouldWait;
        }

        if (resumedFromIdle)
        {
            SERVICE_LOG_DEBUG("capture thread resumed, cameraId: " << mCameraId);
        }

        auto frameStart = std::chrono::steady_clock::now();

        if (auto frame = readImageData())
        {
            mSubscriptions.dispatch(frame);
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
