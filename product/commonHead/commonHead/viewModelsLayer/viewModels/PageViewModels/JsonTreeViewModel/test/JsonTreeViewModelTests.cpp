#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>

#include <commonHead/viewModels/JsonTreeViewModel/IJsonTreeModel.h>
#include <commonHead/viewModels/JsonTreeViewModel/JsonTreeViewModelCreator.h>

TEST_CASE("JsonTreeViewModel creator returns its public API", "[JsonTreeViewModel][Api]")
{
    std::shared_ptr<commonHead::viewModels::IJsonTreeViewModel> viewModel =
        commonHead::viewModels::impl::createJsonTreeViewModel(
            commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "JsonTreeViewModel");
    REQUIRE_NOTHROW(viewModel->initViewModel());
}

TEST_CASE("JsonTreeViewModel parses and navigates a JSON tree", "[JsonTreeViewModel][Api]")
{
    auto viewModel = commonHead::viewModels::impl::createJsonTreeViewModel(
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

TEST_CASE("JsonTreeViewModel reports invalid input and handles empty tree helpers", "[JsonTreeViewModel][Api]")
{
    auto viewModel = commonHead::viewModels::impl::createJsonTreeViewModel(
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
