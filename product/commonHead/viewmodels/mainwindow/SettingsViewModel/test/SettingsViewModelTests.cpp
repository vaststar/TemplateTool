#include <catch2/catch_test_macros.hpp>

#include <commonHead/viewModels/SettingsViewModel/SettingsViewModelCreator.h>

TEST_CASE("SettingsViewModel creator returns its public API", "[SettingsViewModel][Api]")
{
    auto viewModel = commonHead::viewModels::impl::createSettingsViewModel(
        commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "SettingsViewModel");
    REQUIRE_FALSE(viewModel->isSettingsTreeReady());
    REQUIRE(viewModel->getSettingsTree() == nullptr);
    REQUIRE_NOTHROW(viewModel->selectNode("general"));
}

TEST_CASE("SettingsViewModel exposes standalone public types", "[SettingsViewModel][Types]")
{
    commonHead::viewModels::model::SettingsNodeData node;
    REQUIRE(node.nodeId.empty());
    REQUIRE(node.parentId.empty());
    REQUIRE(node.title.empty());
    REQUIRE(node.panelType == commonHead::viewModels::model::SettingsPanelType::None);

    node.nodeId = "general.appearance";
    node.parentId = "general";
    node.panelType = commonHead::viewModels::model::SettingsPanelType::Appearance;
    REQUIRE(node.nodeId == "general.appearance");
    REQUIRE(node.parentId == "general");
    REQUIRE(node.panelType == commonHead::viewModels::model::SettingsPanelType::Appearance);
}
