#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include "MediaService.h"
#include "VideoFrame.h"

TEST_CASE("MediaService can be constructed", "[MediaService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto service = std::make_shared<ucf::service::MediaService>(fakeCoreFramework);
    REQUIRE(service != nullptr);
}

TEST_CASE("MediaService validates complete video frame buffers", "[MediaService][VideoFrame]")
{
    using ucf::service::media::PixelFormat;
    using ucf::service::media::VideoFrame;

    REQUIRE(VideoFrame({1, 2, 3}, 1, 1, 3, PixelFormat::RGB888).isValid());
    REQUIRE(VideoFrame({1, 2, 3, 0, 0, 0, 0, 0},
                       1,
                       2,
                       4,
                       PixelFormat::RGB888).isValid());

    REQUIRE_FALSE(VideoFrame({}, 1, 1, 3, PixelFormat::RGB888).isValid());
    REQUIRE_FALSE(VideoFrame({1, 2, 3}, 1, 1, 0, PixelFormat::RGB888).isValid());
    REQUIRE_FALSE(VideoFrame({1, 2, 3}, 1, 1, -1, PixelFormat::RGB888).isValid());
    REQUIRE_FALSE(VideoFrame({1, 2, 3}, 2, 1, 3, PixelFormat::RGB888).isValid());
    REQUIRE_FALSE(VideoFrame({1, 2}, 1, 1, 3, PixelFormat::RGB888).isValid());
    REQUIRE_FALSE(VideoFrame({1, 2, 3}, 1, 1, 3, PixelFormat::Unknown).isValid());
}
