#include <catch2/catch_test_macros.hpp>

#include <string>

#include <commonHead/viewModels/CameraDirectoryViewModel/CameraDirectoryViewModelCreator.h>

TEST_CASE("CameraDirectoryViewModel creator returns its public API",
          "[CameraDirectoryViewModel][Api]")
{
    auto viewModel = commonHead::viewModels::impl::createCameraDirectoryViewModel(
        commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "CameraDirectoryViewModel");
    REQUIRE_FALSE(viewModel->isCameraDirectoryReady());
    REQUIRE(viewModel->getCameraTree() == nullptr);
    REQUIRE(viewModel->getCurrentCameraId().empty());
    REQUIRE_FALSE(viewModel->getCameraSource("missing").has_value());
    REQUIRE(viewModel->canAddCameraNode(
        "", commonHead::viewModels::model::CameraDirectoryNodeType::Group));
    REQUIRE_FALSE(viewModel->canRemoveCameraNode("missing"));
    REQUIRE_FALSE(viewModel->canMoveCameraNode("missing", ""));
}

TEST_CASE("CameraDirectoryViewModel exposes standalone directory types",
          "[CameraDirectoryViewModel][Types]")
{
    using namespace commonHead::viewModels::model;

    CameraDirectoryNodeData node;
    REQUIRE(node.id.empty());
    REQUIRE(node.displayName.empty());
    REQUIRE(node.type == CameraDirectoryNodeType::Group);
    REQUIRE(node.status == CameraNodeStatus::Active);

    CameraDirectoryRelationData relation{"relation", "parent", "child"};
    REQUIRE(relation.id == "relation");
    REQUIRE(relation.parentId == "parent");
    REQUIRE(relation.childId == "child");
}

TEST_CASE("CameraDirectoryViewModel accepts media types through its API dependency",
          "[CameraDirectoryViewModel][Api][Types]")
{
    using namespace commonHead::viewModels;

    auto viewModel = impl::createCameraDirectoryViewModel(commonHead::ICommonHeadFrameworkWptr{});
    model::CameraSource source = model::NetworkCameraSource{
        "rtsp://camera", "tcp", 1000, 2000};

    viewModel->addCamera("camera", "Camera", source);
    viewModel->updateCamera("camera", "Camera", source);
    viewModel->removeCameras({"camera"});
    viewModel->selectCamera("camera");

    REQUIRE_FALSE(viewModel->isCameraDirectoryReady());
    REQUIRE(viewModel->getCurrentCameraId().empty());
    REQUIRE_FALSE(viewModel->getCameraSource("camera").has_value());
}
