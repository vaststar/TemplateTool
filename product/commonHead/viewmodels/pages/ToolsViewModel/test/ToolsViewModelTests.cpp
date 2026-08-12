#include <catch2/catch_test_macros.hpp>

#include <commonHead/viewModels/ToolsViewModel/ToolsViewModelCreator.h>

TEST_CASE("ToolsViewModel creator returns its public API",
          "[ToolsViewModel][Api]")
{
    using namespace commonHead::viewModels;

    auto viewModel = impl::createToolsViewModel(commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "ToolsViewModel");
    REQUIRE_FALSE(viewModel->isToolsTreeReady());
    REQUIRE(viewModel->getToolsTree() == nullptr);

    const auto encoded = viewModel->base64Encode("hello");
    REQUIRE(encoded.success);
    REQUIRE(encoded.data == "aGVsbG8=");

    const auto decoded = viewModel->base64Decode(encoded.data);
    REQUIRE(decoded.success);
    REQUIRE(decoded.data == "hello");

    const auto formatted = viewModel->jsonFormat("{\"value\":1}");
    REQUIRE(formatted.success);
    REQUIRE_FALSE(formatted.data.empty());

    const auto minified = viewModel->jsonMinify("{ \"value\" : 1 }");
    REQUIRE(minified.success);
    REQUIRE(minified.data == "{\"value\":1}");

    const auto invalidJson = viewModel->jsonValidate("{");
    REQUIRE_FALSE(invalidJson.success);
    REQUIRE_FALSE(invalidJson.errorMessage.empty());

    const auto currentTimestamp = viewModel->getCurrentTimestamp();
    REQUIRE(currentTimestamp.success);
    REQUIRE(currentTimestamp.timestamp > 0);

    const auto uuid = viewModel->generateUuid();
    REQUIRE(viewModel->isValidUuid(uuid));
    REQUIRE_FALSE(viewModel->isValidUuid("not-a-uuid"));
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

    Base64Result base64Result;
    JsonFormatResult jsonResult;
    TimestampResult timestampResult;
    REQUIRE_FALSE(base64Result.success);
    REQUIRE_FALSE(jsonResult.success);
    REQUIRE_FALSE(timestampResult.success);
    REQUIRE(timestampResult.timestamp == 0);
}
