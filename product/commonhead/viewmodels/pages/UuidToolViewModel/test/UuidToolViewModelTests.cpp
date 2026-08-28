#include <catch2/catch_test_macros.hpp>

#include <commonhead/viewmodels/UuidToolViewModel/UuidToolViewModelCreator.h>

TEST_CASE("UuidToolViewModel generates and validates UUIDs", "[UuidToolViewModel]")
{
    auto viewModel = commonHead::viewModels::impl::createUuidToolViewModel(
        commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "UuidToolViewModel");
    REQUIRE_NOTHROW(viewModel->initViewModel());

    const auto uuid = viewModel->generate();
    REQUIRE(viewModel->isValid(uuid));
    REQUIRE_FALSE(viewModel->isValid("not-a-uuid"));
}
