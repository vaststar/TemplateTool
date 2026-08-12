#include <catch2/catch_test_macros.hpp>

#include <commonhead/viewmodels/RecordingViewModel/RecordingViewModelCreator.h>

TEST_CASE("RecordingViewModel creator returns its public API",
          "[RecordingViewModel][Api]")
{
    using namespace commonHead::viewModels;

    auto viewModel = impl::createRecordingViewModel(commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "RecordingViewModel");
    REQUIRE(viewModel->getState() == model::RecordingState::Idle);
    REQUIRE(viewModel->getDuration() == 0);
    REQUIRE_FALSE(viewModel->isFFmpegAvailable());
    REQUIRE(viewModel->getFFmpegPath().empty());

    const auto settings = viewModel->getSettings();
    REQUIRE(settings.outputDirectory.empty());
    REQUIRE(settings.videoFormat == "mp4");
    REQUIRE(settings.framesPerSecond == 30);
    REQUIRE_FALSE(settings.enableMicrophone);
    REQUIRE_FALSE(settings.enableSystemAudio);
    REQUIRE(settings.micDeviceId.empty());
    REQUIRE(settings.systemAudioDeviceId.empty());
}

TEST_CASE("RecordingViewModel exposes standalone recording types",
          "[RecordingViewModel][Types]")
{
    using namespace commonHead::viewModels::model;

    AudioDeviceInfo device;
    REQUIRE(device.id.empty());
    REQUIRE(device.displayName.empty());
    REQUIRE(device.isInput);
    REQUIRE(device.deviceType == AudioDeviceType::Microphone);

    RecordingSettings settings;
    REQUIRE(settings.outputDirectory.empty());
    REQUIRE(settings.videoFormat == "mp4");
    REQUIRE(settings.framesPerSecond == 30);
    REQUIRE_FALSE(settings.enableMicrophone);
    REQUIRE_FALSE(settings.enableSystemAudio);
    REQUIRE(settings.micDeviceId.empty());
    REQUIRE(settings.systemAudioDeviceId.empty());
}
