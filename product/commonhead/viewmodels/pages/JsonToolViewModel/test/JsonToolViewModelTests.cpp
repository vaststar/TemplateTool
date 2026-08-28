#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>

#include <commonhead/viewmodels/JsonToolViewModel/IJsonTreeModel.h>
#include <commonhead/viewmodels/JsonToolViewModel/JsonToolViewModelCreator.h>

TEST_CASE("JsonToolViewModel creator returns its public API", "[JsonToolViewModel][Api]")
{
    std::shared_ptr<commonHead::viewModels::IJsonToolViewModel> viewModel =
        commonHead::viewModels::impl::createJsonToolViewModel(
            commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "JsonToolViewModel");
    REQUIRE_NOTHROW(viewModel->initViewModel());
}

TEST_CASE("JsonToolViewModel formats and validates JSON", "[JsonToolViewModel][Api]")
{
    auto viewModel = commonHead::viewModels::impl::createJsonToolViewModel(
        commonHead::ICommonHeadFrameworkWptr{});

    const auto formatted = viewModel->format("{\"value\":1}");
    REQUIRE(formatted.success);
    REQUIRE_FALSE(formatted.data.empty());

    const auto minified = viewModel->minify("{ \"value\" : 1 }");
    REQUIRE(minified.success);
    REQUIRE(minified.data == "{\"value\":1}");

    const auto invalid = viewModel->validate("{");
    REQUIRE_FALSE(invalid.success);
    REQUIRE_FALSE(invalid.errorMessage.empty());
}

TEST_CASE("JsonToolViewModel parses and navigates a JSON tree", "[JsonToolViewModel][Api]")
{
    auto viewModel = commonHead::viewModels::impl::createJsonToolViewModel(
        commonHead::ICommonHeadFrameworkWptr{});
    auto tree = viewModel->parseJsonTree(R"({"name":"codex","enabled":true})");

    REQUIRE(tree != nullptr);
    REQUIRE(viewModel->getLastError().empty());

    const auto virtualRoot = tree->getRoot();
    REQUIRE(virtualRoot != nullptr);
    REQUIRE(virtualRoot->getChildCount() == 1);

    const auto objectNode = virtualRoot->getChild(0);
    REQUIRE(objectNode != nullptr);
    REQUIRE(objectNode->getNodeData().type == commonHead::viewModels::model::JsonNodeType::Object);

    commonHead::viewModels::model::JsonTreeNodePtr nameNode;
    for (std::size_t index = 0; index < objectNode->getChildCount(); ++index)
    {
        const auto child = objectNode->getChild(index);
        if (child && child->getNodeData().key == "name")
        {
            nameNode = child;
            break;
        }
    }

    REQUIRE(nameNode != nullptr);
    REQUIRE(viewModel->getJsonPath(tree, nameNode.get()) == "$.name");
    REQUIRE(viewModel->getSubTreeJson(tree, nameNode.get()) == R"("codex")");
}

TEST_CASE("JsonToolViewModel reports invalid input and handles empty tree helpers", "[JsonToolViewModel][Api]")
{
    auto viewModel = commonHead::viewModels::impl::createJsonToolViewModel(
        commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel->parseJsonTree("{") == nullptr);
    REQUIRE_FALSE(viewModel->getLastError().empty());
    REQUIRE(viewModel->getJsonPath({}, nullptr) == "$");
    REQUIRE(viewModel->getSubTreeJson({}, nullptr).empty());

    const commonHead::viewModels::model::JsonNodeData defaultData;
    REQUIRE(defaultData.type == commonHead::viewModels::model::JsonNodeType::Null);
    REQUIRE(defaultData.key.empty());
    REQUIRE(defaultData.displayValue.empty());
    REQUIRE(defaultData.childCount == 0);
}
