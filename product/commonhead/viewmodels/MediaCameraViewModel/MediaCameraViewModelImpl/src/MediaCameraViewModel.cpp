#include "MediaCameraViewModel.h"
#include "LoggerDefine.h"

#include <type_traits>
#include <variant>

#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonhead/ServiceLocator/IServiceLocator.h>
#include <commonhead/viewmodels/MediaCameraViewModel/MediaCameraViewModelCreator.h>
#include <ucf/services/MediaService/IMediaService.h>
#include <ucf/services/MediaService/MediaTypes.h>

namespace commonHead::viewModels {

namespace {

ucf::service::media::CameraSource toServiceSource(const model::CameraSource& source)
{
    return std::visit(
        [](const auto& cameraSource) -> ucf::service::media::CameraSource {
            using SourceType = std::decay_t<decltype(cameraSource)>;
            if constexpr (std::is_same_v<SourceType, model::LocalCameraSource>)
            {
                return ucf::service::media::LocalCameraSource{cameraSource.index};
            }
            else
            {
                return ucf::service::media::NetworkCameraSource{
                    cameraSource.url,
                    cameraSource.transport,
                    cameraSource.openTimeoutMs,
                    cameraSource.readTimeoutMs};
            }
        },
        source);
}

} // namespace

namespace impl {

std::shared_ptr<IMediaCameraViewModel> createMediaCameraViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<MediaCameraViewModel>(commonHeadFramework);
}

} // namespace impl

MediaCameraViewModel::MediaCameraViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : IMediaCameraViewModel(commonHeadFramework)
    , mWorker(&MediaCameraViewModel::workerLoop, this)
{
    MEDIA_CAMERA_VIEW_MODEL_LOG_DEBUG("create MediaCameraViewModel");
}

MediaCameraViewModel::~MediaCameraViewModel()
{
    MEDIA_CAMERA_VIEW_MODEL_LOG_DEBUG("");

    mOpenRequested = false;
    ++mOperationGeneration;
    {
        std::scoped_lock lock(mCommandMutex);
        mStopping = true;
        mCommands.clear();
    }
    mCommandCondition.notify_all();
    if (mWorker.joinable())
    {
        mWorker.join();
    }
}

std::string MediaCameraViewModel::getViewModelName() const
{
    return "MediaCameraViewModel";
}

void MediaCameraViewModel::init()
{
}

void MediaCameraViewModel::openCamera(const model::CameraSource& source)
{
    bool expected = false;
    if (!mOpenRequested.compare_exchange_strong(expected, true))
    {
        MEDIA_CAMERA_VIEW_MODEL_LOG_DEBUG("ignore open request while camera is opening or opened");
        return;
    }

    const auto generation = ++mOperationGeneration;
    {
        std::scoped_lock lock(mCommandMutex);
        if (mStopping)
        {
            mOpenRequested = false;
            return;
        }
        mCommands.push_back(Command{CommandType::Open, generation, source});
    }
    mCommandCondition.notify_one();
}

void MediaCameraViewModel::closeCamera()
{
    mOpenRequested = false;
    const auto generation = ++mOperationGeneration;
    {
        std::scoped_lock lock(mCommandMutex);
        if (mStopping)
        {
            return;
        }
        mCommands.push_back(Command{CommandType::Close, generation, std::nullopt});
    }
    mCommandCondition.notify_one();
}

bool MediaCameraViewModel::isOpened() const
{
    std::scoped_lock lock(mSessionMutex);
    return !mCameraId.empty() && !mSubscriptionId.empty();
}

void MediaCameraViewModel::workerLoop()
{
    while (true)
    {
        Command command{CommandType::Close, 0, std::nullopt};
        {
            std::unique_lock lock(mCommandMutex);
            mCommandCondition.wait(lock, [this] {
                return mStopping || !mCommands.empty();
            });

            if (mStopping && mCommands.empty())
            {
                break;
            }

            command = std::move(mCommands.front());
            mCommands.pop_front();
        }

        if (command.type == CommandType::Open && command.source)
        {
            processOpen(command.generation, *command.source);
        }
        else
        {
            closeSessionOnWorker();
        }
    }

    closeSessionOnWorker();
}

void MediaCameraViewModel::processOpen(
    std::uint64_t generation,
    const model::CameraSource& source)
{
    const auto isCurrent = [this, generation] {
        return generation == mOperationGeneration.load() && mOpenRequested.load();
    };

    if (!isCurrent())
    {
        return;
    }

    auto mediaService = getMediaService();
    if (!mediaService)
    {
        notifyOpenFailedIfCurrent(generation);
        return;
    }

    auto cameraId = mediaService->openCamera(toServiceSource(source));
    if (!isCurrent())
    {
        if (!cameraId.empty())
        {
            mediaService->releaseCamera(cameraId);
        }
        return;
    }

    if (cameraId.empty())
    {
        notifyOpenFailedIfCurrent(generation);
        return;
    }

    const auto weakSelf = weak_from_this();
    auto subscriptionId = mediaService->startVideoCapture(
        cameraId,
        [weakSelf](const ucf::service::media::IVideoFramePtr& frame) {
            if (auto self = weakSelf.lock())
            {
                if (auto convertedFrame = self->convertServiceFrameToViewModelFrame(frame))
                {
                    self->fireNotification(
                        &IMediaCameraViewModelCallback::onCameraFrameReceived,
                        *convertedFrame);
                }
            }
        });

    if (!isCurrent())
    {
        if (!subscriptionId.empty())
        {
            mediaService->stopVideoCapture(cameraId, subscriptionId);
        }
        mediaService->releaseCamera(cameraId);
        return;
    }

    if (subscriptionId.empty())
    {
        mediaService->releaseCamera(cameraId);
        notifyOpenFailedIfCurrent(generation);
        return;
    }

    {
        std::scoped_lock lock(mSessionMutex);
        mCameraId = std::move(cameraId);
        mSubscriptionId = std::move(subscriptionId);
    }

    // A close can invalidate the operation immediately after publication.
    // Clean up here as well as via the queued Close command.
    if (!isCurrent())
    {
        closeSessionOnWorker();
    }
}

void MediaCameraViewModel::closeSessionOnWorker()
{
    std::string cameraId;
    std::string subscriptionId;
    {
        std::scoped_lock lock(mSessionMutex);
        cameraId.swap(mCameraId);
        subscriptionId.swap(mSubscriptionId);
    }

    if (cameraId.empty())
    {
        return;
    }

    if (auto mediaService = getMediaService())
    {
        if (!subscriptionId.empty())
        {
            mediaService->stopVideoCapture(cameraId, subscriptionId);
        }
        mediaService->releaseCamera(cameraId);
    }
}

void MediaCameraViewModel::notifyOpenFailedIfCurrent(std::uint64_t generation)
{
    if (generation != mOperationGeneration.load())
    {
        return;
    }

    bool expected = true;
    if (mOpenRequested.compare_exchange_strong(expected, false))
    {
        fireNotification(&IMediaCameraViewModelCallback::onCameraOpenFailed);
    }
}

std::shared_ptr<ucf::service::IMediaService> MediaCameraViewModel::getMediaService() const
{
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            return serviceLocator->getMediaService().lock();
        }
    }
    return nullptr;
}

std::optional<model::VideoFrame> MediaCameraViewModel::convertServiceFrameToViewModelFrame(
    const ucf::service::media::IVideoFramePtr& frame) const
{
    if (!frame || !frame->isValid() || !frame->getData())
    {
        return std::nullopt;
    }

    model::VideoFrame convertedFrame{
        std::vector<uint8_t>(frame->getData(), frame->getData() + frame->getDataSize()),
        frame->getWidth(),
        frame->getHeight(),
        frame->getBytesPerLine(),
        static_cast<model::PixelFormat>(frame->getFormat())};

    if (!convertedFrame.isValid())
    {
        return std::nullopt;
    }
    return convertedFrame;
}

} // namespace commonHead::viewModels
