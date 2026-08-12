#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>
#include <vector>

#include <commonHead/viewModels/MediaCameraViewModel/MediaCameraViewModelCreator.h>

namespace {

class CapturingMediaCameraViewModelCallback final
    : public commonHead::viewModels::IMediaCameraViewModelCallback
{
public:
    void onCameraOpenFailed() override
    {
        openFailed = true;
    }

    bool openFailed = false;
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
}

TEST_CASE("MediaCameraViewModel reports an unavailable framework", "[MediaCameraViewModel][Api]")
{
    using namespace commonHead::viewModels;

    auto viewModel = impl::createMediaCameraViewModel(commonHead::ICommonHeadFrameworkWptr{});
    auto callback = std::make_shared<CapturingMediaCameraViewModelCallback>();
    viewModel->registerCallback(callback);

    viewModel->openCamera(model::LocalCameraSource{0});
    REQUIRE(callback->openFailed);
    REQUIRE_FALSE(viewModel->isOpened());

    viewModel->unRegisterCallback(callback);
}
