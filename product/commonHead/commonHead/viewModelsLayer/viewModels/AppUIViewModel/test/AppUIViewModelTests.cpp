#include <catch2/catch_test_macros.hpp>

#include <memory>

#include <commonHead/viewModels/AppUIViewModel/AppUIViewModelCreator.h>

namespace {

class NoOpAppUIViewModelCallback final
    : public commonHead::viewModels::IAppUIViewModelCallback
{
public:
    void onAppConfigInitialized() override {}
};

} // namespace

TEST_CASE("AppUIViewModel creator returns its public API", "[AppUIViewModel][Api]")
{
    std::shared_ptr<commonHead::viewModels::IAppUIViewModel> viewModel =
        commonHead::viewModels::impl::createAppUIViewModel(
            commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "AppUIViewModel");
}

TEST_CASE("AppUIViewModel public API is safe without a framework", "[AppUIViewModel][Api]")
{
    auto viewModel = commonHead::viewModels::impl::createAppUIViewModel(
        commonHead::ICommonHeadFrameworkWptr{});
    auto callback = std::make_shared<NoOpAppUIViewModelCallback>();

    REQUIRE_NOTHROW(viewModel->registerCallback(callback));
    REQUIRE_NOTHROW(viewModel->initViewModel());
    REQUIRE_NOTHROW(viewModel->initApplication());
    REQUIRE_NOTHROW(viewModel->unRegisterCallback(callback));
}
