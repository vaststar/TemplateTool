#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonhead/viewmodels/MediaCameraViewModel/MediaCameraViewModelCreator.h>
#include <ucf/CoreFramework/CoreFrameworkCreator.h>
#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/services/MediaService/IMediaService.h>
#include <ucf/utilities/NotificationHelper/NotificationHelper.h>

namespace {

class CapturingMediaCameraViewModelCallback final
    : public commonHead::viewModels::IMediaCameraViewModelCallback
{
public:
    void onCameraOpenFailed() override
    {
        {
            std::scoped_lock lock(mMutex);
            ++mOpenFailedCount;
        }
        mChanged.notify_all();
    }

    bool waitForOpenFailed()
    {
        std::unique_lock lock(mMutex);
        return mChanged.wait_for(lock, std::chrono::seconds(2), [this] {
            return mOpenFailedCount > 0;
        });
    }

    std::size_t openFailedCount() const
    {
        std::scoped_lock lock(mMutex);
        return mOpenFailedCount;
    }

private:
    mutable std::mutex mMutex;
    std::condition_variable mChanged;
    std::size_t mOpenFailedCount{0};
};

class ControllableMediaService final
    : public ucf::service::IMediaService
    , public ucf::utilities::NotificationHelper<ucf::service::IMediaServiceCallback>
{
public:
    std::string getServiceName() const override { return "ControllableMediaService"; }

    std::string openCamera(const ucf::service::media::CameraSource&) override
    {
        std::unique_lock lock(mMutex);
        mEvents.emplace_back("open");
        mOpenEntered = true;
        mChanged.notify_all();
        mChanged.wait(lock, [this] { return !mBlockOpen; });
        return "camera-id";
    }

    void releaseCamera(const std::string&) override
    {
        recordEvent("release");
    }

    std::vector<std::string> getOpenedCameras() const override { return {}; }

    ucf::service::media::IVideoFramePtr readImageData(const std::string&) override
    {
        return nullptr;
    }

    std::string startVideoCapture(
        const std::string&,
        ucf::service::VideoFrameCallback) override
    {
        recordEvent("start");
        return "subscription-id";
    }

    void stopVideoCapture(const std::string&, const std::string&) override
    {
        recordEvent("stop");
    }

    void blockOpen()
    {
        std::scoped_lock lock(mMutex);
        mBlockOpen = true;
    }

    void allowOpen()
    {
        {
            std::scoped_lock lock(mMutex);
            mBlockOpen = false;
        }
        mChanged.notify_all();
    }

    bool waitForOpenEntered()
    {
        std::unique_lock lock(mMutex);
        return mChanged.wait_for(lock, std::chrono::seconds(2), [this] {
            return mOpenEntered;
        });
    }

    bool waitForEvent(const std::string& event)
    {
        std::unique_lock lock(mMutex);
        return mChanged.wait_for(lock, std::chrono::seconds(2), [this, &event] {
            return std::find(mEvents.begin(), mEvents.end(), event) != mEvents.end();
        });
    }

    std::vector<std::string> events() const
    {
        std::scoped_lock lock(mMutex);
        return mEvents;
    }

protected:
    void initService() override {}
    void deinitService() override {}

private:
    void recordEvent(std::string event)
    {
        {
            std::scoped_lock lock(mMutex);
            mEvents.push_back(std::move(event));
        }
        mChanged.notify_all();
    }

private:
    mutable std::mutex mMutex;
    std::condition_variable mChanged;
    std::vector<std::string> mEvents;
    bool mBlockOpen{false};
    bool mOpenEntered{false};
};

struct MediaCameraViewModelFixture
{
    MediaCameraViewModelFixture()
        : coreFramework(ucf::framework::createCoreFramework())
        , mediaService(std::make_shared<ControllableMediaService>())
    {
        coreFramework->registerService<ucf::service::IMediaService>(mediaService);
        commonHeadFramework = commonHead::ICommonHeadFramework::createInstance(coreFramework);
        viewModel = commonHead::viewModels::impl::createMediaCameraViewModel(commonHeadFramework);
    }

    ucf::framework::ICoreFrameworkPtr coreFramework;
    std::shared_ptr<ControllableMediaService> mediaService;
    std::shared_ptr<commonHead::ICommonHeadFramework> commonHeadFramework;
    std::shared_ptr<commonHead::viewModels::IMediaCameraViewModel> viewModel;
};

} // namespace

TEST_CASE("MediaCameraViewModel creator returns its public API", "[MediaCameraViewModel][Api]")
{
    auto viewModel = commonHead::viewModels::impl::createMediaCameraViewModel(
        commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "MediaCameraViewModel");
    REQUIRE_FALSE(viewModel->isOpened());
}

TEST_CASE("MediaCameraViewModel exposes standalone camera source types", "[MediaCameraViewModel][Types]")
{
    using namespace commonHead::viewModels::model;

    CameraSource localSource = LocalCameraSource{3};
    REQUIRE(toDisplayString(localSource) == "local:3");

    CameraSource networkSource = NetworkCameraSource{"rtsp://camera", "tcp", 1000, 2000};
    REQUIRE(toDisplayString(networkSource) == "url:rtsp://camera");
}

TEST_CASE("MediaCameraViewModel exposes standalone video frame types", "[MediaCameraViewModel][Types]")
{
    using namespace commonHead::viewModels::model;

    VideoFrame emptyFrame;
    REQUIRE_FALSE(emptyFrame.isValid());

    VideoFrame frame{{1, 2, 3, 4}, 1, 1, 4, PixelFormat::RGBA8888};
    REQUIRE(frame.isValid());
    REQUIRE(frame.width == 1);
    REQUIRE(frame.height == 1);
    REQUIRE(frame.bytesPerLine == 4);
    REQUIRE(frame.format == PixelFormat::RGBA8888);

    REQUIRE_FALSE(VideoFrame{{1, 2, 3}, 1, 1, 0, PixelFormat::RGB888}.isValid());
    REQUIRE_FALSE(VideoFrame{{1, 2, 3}, 1, 1, -1, PixelFormat::RGB888}.isValid());
    REQUIRE_FALSE(VideoFrame{{1, 2, 3}, 2, 1, 3, PixelFormat::RGB888}.isValid());
    REQUIRE_FALSE(VideoFrame{{1, 2}, 1, 1, 3, PixelFormat::RGB888}.isValid());
    REQUIRE_FALSE(VideoFrame{{1, 2, 3}, 1, 1, 3, PixelFormat::Unknown}.isValid());
    REQUIRE(VideoFrame{{1, 2, 3, 0, 0, 0, 0, 0},
                       1,
                       2,
                       4,
                       PixelFormat::RGB888}.isValid());
}

TEST_CASE("MediaCameraViewModel reports an unavailable framework", "[MediaCameraViewModel][Api]")
{
    using namespace commonHead::viewModels;

    auto viewModel = impl::createMediaCameraViewModel(commonHead::ICommonHeadFrameworkWptr{});
    auto callback = std::make_shared<CapturingMediaCameraViewModelCallback>();
    viewModel->registerCallback(callback);

    viewModel->openCamera(model::LocalCameraSource{0});
    REQUIRE(callback->waitForOpenFailed());
    REQUIRE(callback->openFailedCount() == 1);
    REQUIRE_FALSE(viewModel->isOpened());

    viewModel->unRegisterCallback(callback);
}

TEST_CASE("MediaCameraViewModel cancels an in-flight open before capture starts", "[MediaCameraViewModel][Concurrency]")
{
    MediaCameraViewModelFixture fixture;
    auto callback = std::make_shared<CapturingMediaCameraViewModelCallback>();
    fixture.viewModel->registerCallback(callback);
    fixture.mediaService->blockOpen();

    fixture.viewModel->openCamera(commonHead::viewModels::model::LocalCameraSource{0});
    REQUIRE(fixture.mediaService->waitForOpenEntered());

    fixture.viewModel->closeCamera();
    fixture.mediaService->allowOpen();

    REQUIRE(fixture.mediaService->waitForEvent("release"));
    const auto events = fixture.mediaService->events();
    REQUIRE(std::find(events.begin(), events.end(), "start") == events.end());
    REQUIRE(callback->openFailedCount() == 0);
    REQUIRE_FALSE(fixture.viewModel->isOpened());
}

TEST_CASE("MediaCameraViewModel ignores a duplicate open request", "[MediaCameraViewModel][Concurrency]")
{
    MediaCameraViewModelFixture fixture;
    fixture.mediaService->blockOpen();

    fixture.viewModel->openCamera(commonHead::viewModels::model::LocalCameraSource{0});
    REQUIRE(fixture.mediaService->waitForOpenEntered());
    fixture.viewModel->openCamera(commonHead::viewModels::model::LocalCameraSource{1});

    fixture.mediaService->allowOpen();
    REQUIRE(fixture.mediaService->waitForEvent("start"));
    fixture.viewModel->closeCamera();
    REQUIRE(fixture.mediaService->waitForEvent("release"));

    const auto events = fixture.mediaService->events();
    REQUIRE(std::count(events.begin(), events.end(), "open") == 1);
    REQUIRE(std::count(events.begin(), events.end(), "start") == 1);
    REQUIRE(std::count(events.begin(), events.end(), "release") == 1);
}

TEST_CASE("MediaCameraViewModel closes capture before releasing the camera", "[MediaCameraViewModel][Lifecycle]")
{
    MediaCameraViewModelFixture fixture;

    fixture.viewModel->openCamera(commonHead::viewModels::model::LocalCameraSource{0});
    REQUIRE(fixture.mediaService->waitForEvent("start"));

    fixture.viewModel->closeCamera();
    fixture.viewModel->closeCamera();
    REQUIRE(fixture.mediaService->waitForEvent("release"));

    fixture.viewModel.reset();
    const auto events = fixture.mediaService->events();
    const auto stop = std::find(events.begin(), events.end(), "stop");
    const auto release = std::find(events.begin(), events.end(), "release");
    REQUIRE(stop != events.end());
    REQUIRE(release != events.end());
    REQUIRE(stop < release);
    REQUIRE(std::count(events.begin(), events.end(), "release") == 1);
}

TEST_CASE("MediaCameraViewModel destructor closes an active session", "[MediaCameraViewModel][Lifecycle]")
{
    MediaCameraViewModelFixture fixture;

    fixture.viewModel->openCamera(commonHead::viewModels::model::LocalCameraSource{0});
    REQUIRE(fixture.mediaService->waitForEvent("start"));

    fixture.viewModel.reset();
    REQUIRE(fixture.mediaService->waitForEvent("release"));

    const auto events = fixture.mediaService->events();
    const auto stop = std::find(events.begin(), events.end(), "stop");
    const auto release = std::find(events.begin(), events.end(), "release");
    REQUIRE(stop != events.end());
    REQUIRE(release != events.end());
    REQUIRE(stop < release);
}
