#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

#include <commonhead/utilities/VMNotificationHelper/VMNotificationHelper.h>
#include <commonhead/viewmodels/MediaCameraViewModel/IMediaCameraViewModel.h>
#include <ucf/services/MediaService/IMediaService.h>
#include <ucf/services/MediaService/MediaTypes.h>

namespace commonHead::viewModels {

class MediaCameraViewModel
    : public virtual IMediaCameraViewModel
    , public virtual commonHead::utilities::VMNotificationHelper<IMediaCameraViewModelCallback>
    , public std::enable_shared_from_this<MediaCameraViewModel>
{
public:
    explicit MediaCameraViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    MediaCameraViewModel() = delete;
    MediaCameraViewModel(const MediaCameraViewModel&) = delete;
    MediaCameraViewModel(MediaCameraViewModel&&) = delete;
    MediaCameraViewModel& operator=(const MediaCameraViewModel&) = delete;
    MediaCameraViewModel& operator=(MediaCameraViewModel&&) = delete;
    ~MediaCameraViewModel() override;

public:
    std::string getViewModelName() const override;
    void openCamera(const model::CameraSource& source) override;
    void closeCamera() override;
    [[nodiscard]] bool isOpened() const override;

protected:
    void init() override;

private:
    enum class CommandType
    {
        Open,
        Close
    };

    struct Command
    {
        CommandType type;
        std::uint64_t generation;
        std::optional<model::CameraSource> source;
    };

    void workerLoop();
    void processOpen(std::uint64_t generation, const model::CameraSource& source);
    void closeSessionOnWorker();
    void notifyOpenFailedIfCurrent(std::uint64_t generation);
    [[nodiscard]] std::shared_ptr<ucf::service::IMediaService> getMediaService() const;
    [[nodiscard]] std::optional<model::VideoFrame> convertServiceFrameToViewModelFrame(
        const ucf::service::media::IVideoFramePtr& frame) const;

private:
    std::atomic<std::uint64_t> mOperationGeneration{0};
    std::atomic<bool> mOpenRequested{false};

    std::mutex mCommandMutex;
    std::condition_variable mCommandCondition;
    std::deque<Command> mCommands;
    bool mStopping{false};

    mutable std::mutex mSessionMutex;
    std::string mCameraId;
    std::string mSubscriptionId;

    // Declared last so synchronization state is initialized before the worker
    // starts and remains alive until the worker has been joined.
    std::jthread mWorker;
};

} // namespace commonHead::viewModels
