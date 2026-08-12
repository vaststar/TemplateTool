#include <catch2/catch_test_macros.hpp>

#include <commonHead/viewModels/MiniAppRuntimeViewModel/MiniAppRuntimeViewModelCreator.h>

TEST_CASE("MiniAppRuntimeViewModel creator returns its public API",
          "[MiniAppRuntimeViewModel][Api]")
{
    using namespace commonHead::viewModels;

    auto viewModel = impl::createMiniAppRuntimeViewModel(commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "MiniAppRuntimeViewModel");
    REQUIRE_FALSE(viewModel->isReady());
    REQUIRE(viewModel->nativeHostHandle() == 0);

    viewModel->start("missing");
    REQUIRE_FALSE(viewModel->isReady());
    REQUIRE(viewModel->nativeHostHandle() == 0);

    viewModel->stop();
    viewModel->stop();
    REQUIRE_FALSE(viewModel->isReady());
}
