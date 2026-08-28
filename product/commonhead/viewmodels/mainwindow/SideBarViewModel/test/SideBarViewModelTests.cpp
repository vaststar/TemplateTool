#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <vector>

#include <commonhead/viewmodels/SideBarViewModel/SideBarViewModelCreator.h>

namespace {

class CapturingSideBarViewModelCallback final
    : public commonHead::viewModels::ISideBarViewModelCallback
{
public:
    void onSubMenuRequested(
        commonHead::viewModels::model::PageId pageId,
        const std::vector<commonHead::viewModels::model::SubMenuItem>& items) override
    {
        requestedPageId = pageId;
        requestedItems = items;
    }

    void onMenuActionClicked(commonHead::viewModels::model::MenuActionId actionId) override
    {
        clickedActionId = actionId;
    }

    commonHead::viewModels::model::PageId requestedPageId{
        commonHead::viewModels::model::PageId::Unknown};
    std::vector<commonHead::viewModels::model::SubMenuItem> requestedItems;
    commonHead::viewModels::model::MenuActionId clickedActionId{
        commonHead::viewModels::model::MenuActionId::Unknown};
};

} // namespace

TEST_CASE("SideBarViewModel creator returns its public API", "[SideBarViewModel][Api]")
{
    auto viewModel = commonHead::viewModels::impl::createSideBarViewModel(
        commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "SideBarViewModel");
    REQUIRE_FALSE(viewModel->isSideBarReady());
    REQUIRE(viewModel->getNavItems().empty());
    REQUIRE(viewModel->getDefaultPageId() == commonHead::viewModels::model::PageId::Home);
}

TEST_CASE("SideBarViewModel exposes standalone public types", "[SideBarViewModel][Types]")
{
    commonHead::viewModels::model::NavItemData item;
    REQUIRE(item.pageId == commonHead::viewModels::model::PageId::Unknown);
    REQUIRE(item.isEnabled());
    REQUIRE(item.isVisible());
    REQUIRE(item.isTopItem());
    REQUIRE_FALSE(item.hasSubMenu());

    item.state = commonHead::viewModels::model::NavItemState::Hidden;
    item.position = commonHead::viewModels::model::NavItemPosition::Bottom;
    item.subMenuItems.push_back({commonHead::viewModels::model::MenuActionId::About, "About"});
    REQUIRE_FALSE(item.isVisible());
    REQUIRE(item.isBottomItem());
    REQUIRE(item.hasSubMenu());
}

TEST_CASE("SideBarViewModel forwards valid menu actions", "[SideBarViewModel][Api]")
{
    auto viewModel = commonHead::viewModels::impl::createSideBarViewModel(
        commonHead::ICommonHeadFrameworkWptr{});
    auto callback = std::make_shared<CapturingSideBarViewModelCallback>();

    viewModel->registerCallback(callback);
    viewModel->handleSubMenuAction(commonHead::viewModels::model::MenuActionId::About);
    REQUIRE(callback->clickedActionId == commonHead::viewModels::model::MenuActionId::About);

    viewModel->handleSubMenuAction(commonHead::viewModels::model::MenuActionId::Unknown);
    REQUIRE(callback->clickedActionId == commonHead::viewModels::model::MenuActionId::About);
    REQUIRE_FALSE(viewModel->navigateTo(commonHead::viewModels::model::PageId::Unknown));
    viewModel->unRegisterCallback(callback);
}

TEST_CASE("SideBarViewModel safely handles a missing framework",
          "[SideBarViewModel][Safety]")
{
    auto viewModel = commonHead::viewModels::impl::createSideBarViewModel(
        commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE_NOTHROW(viewModel->initViewModel());
    REQUIRE_FALSE(viewModel->isSideBarReady());
    REQUIRE(viewModel->getNavItems().empty());

    REQUIRE_NOTHROW(viewModel->reloadNavConfig());
    REQUIRE_FALSE(viewModel->isSideBarReady());
    REQUIRE(viewModel->getNavItems().empty());
}
