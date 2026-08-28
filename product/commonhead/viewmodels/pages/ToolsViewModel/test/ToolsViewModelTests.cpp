#include <catch2/catch_test_macros.hpp>

#include <commonhead/viewmodels/ToolsViewModel/ToolsViewModelCreator.h>

TEST_CASE("ToolsViewModel creator returns its public API",
          "[ToolsViewModel][Api]")
{
    using namespace commonHead::viewModels;

    auto viewModel = impl::createToolsViewModel(commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "ToolsViewModel");
    REQUIRE_FALSE(viewModel->isToolsTreeReady());
    REQUIRE(viewModel->getToolsTree() == nullptr);
}

TEST_CASE("ToolsViewModel exposes standalone types",
          "[ToolsViewModel][Types]")
{
    using namespace commonHead::viewModels;
    using namespace commonHead::viewModels::model;

    ToolNodeData node;
    REQUIRE(node.nodeId.empty());
    REQUIRE(node.parentId.empty());
    REQUIRE(node.title.empty());
    REQUIRE(node.icon.empty());
    REQUIRE(node.panelType == ToolPanelType::None);

    ToolsTreeNodePtr treeNode;
    ToolsTreePtr tree;
    REQUIRE(treeNode == nullptr);
    REQUIRE(tree == nullptr);
}

TEST_CASE(
    "ToolsViewModel safely handles missing framework for resource-backed operations",
    "[ToolsViewModel][Safety]")
{
    using namespace commonHead::viewModels;

    auto viewModel = impl::createToolsViewModel(
        commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE_NOTHROW(viewModel->initViewModel());
    REQUIRE_FALSE(viewModel->isToolsTreeReady());
    REQUIRE(viewModel->getToolsTree() == nullptr);

    REQUIRE_NOTHROW(viewModel->reloadTree());
    REQUIRE_FALSE(viewModel->isToolsTreeReady());
}
