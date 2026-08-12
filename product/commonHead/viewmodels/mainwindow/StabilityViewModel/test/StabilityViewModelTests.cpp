#include <catch2/catch_test_macros.hpp>

#include <memory>

#include <commonHead/viewModels/StabilityViewModel/StabilityViewModelCreator.h>

namespace {

class NoOpStabilityViewModelCallback final
    : public commonHead::viewModels::IStabilityViewModelCallback
{
};

} // namespace

TEST_CASE("StabilityViewModel creator returns its public API", "[StabilityViewModel][Api]")
{
    std::shared_ptr<commonHead::viewModels::IStabilityViewModel> viewModel =
        commonHead::viewModels::impl::createStabilityViewModel(
            commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "StabilityViewModel");
    REQUIRE(viewModel->getHeartbeatIntervalMs() == 1000);
}

TEST_CASE("StabilityViewModel public API is safe without a framework", "[StabilityViewModel][Api]")
{
    auto viewModel = commonHead::viewModels::impl::createStabilityViewModel(
        commonHead::ICommonHeadFrameworkWptr{});
    auto callback = std::make_shared<NoOpStabilityViewModelCallback>();

    REQUIRE_NOTHROW(viewModel->registerCallback(callback));
    REQUIRE_NOTHROW(viewModel->initViewModel());
    REQUIRE_NOTHROW(viewModel->reportHeartbeat());
    REQUIRE_NOTHROW(viewModel->unRegisterCallback(callback));
}
