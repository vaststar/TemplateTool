#include <catch2/catch_test_macros.hpp>

#include <commonHead/viewModels/ScreenshotViewModel/ScreenshotViewModelCreator.h>

TEST_CASE("ScreenshotViewModel creator returns its public API",
          "[ScreenshotViewModel][Api]")
{
    using namespace commonHead::viewModels;

    auto viewModel = impl::createScreenshotViewModel(commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "ScreenshotViewModel");
    REQUIRE(viewModel->getState() == model::ScreenshotState::Idle);
    REQUIRE(viewModel->getAnnotations().empty());
    REQUIRE_FALSE(viewModel->canUndo());
    REQUIRE_FALSE(viewModel->canRedo());
    REQUIRE(viewModel->getBase64Png().empty());

    const auto settings = viewModel->getSettings();
    REQUIRE(settings.outputDirectory.empty());
    REQUIRE(settings.imageFormat == "png");
    REQUIRE(settings.jpegQuality == 90);
    REQUIRE(settings.captureDelay == 0);
    REQUIRE_FALSE(settings.addTimestamp);
}

TEST_CASE("ScreenshotViewModel exposes standalone screenshot types",
          "[ScreenshotViewModel][Types]")
{
    using namespace commonHead::viewModels::model;

    ScreenshotSettings settings;
    REQUIRE(settings.imageFormat == "png");
    REQUIRE(settings.jpegQuality == 90);
    REQUIRE(settings.captureDelay == 0);
    REQUIRE_FALSE(settings.addTimestamp);

    AnnotationData annotation;
    REQUIRE(annotation.id == 0);
    REQUIRE(annotation.r == 255);
    REQUIRE(annotation.g == 0);
    REQUIRE(annotation.b == 0);
    REQUIRE(annotation.a == 255);
    REQUIRE(annotation.thickness == 2);
    REQUIRE(annotation.fontSize == 16);
    REQUIRE(annotation.mosaicBlockSize == 10);

    DisplayInfoVM display;
    REQUIRE(display.displayId == 0);
    REQUIRE(display.scaleFactor == 1);
    REQUIRE_FALSE(display.isPrimary);
}
