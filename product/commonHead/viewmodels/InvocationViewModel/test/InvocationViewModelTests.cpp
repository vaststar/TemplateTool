#include <catch2/catch_test_macros.hpp>

#include <memory>

#include <commonHead/viewModels/InvocationViewModel/InvocationViewModelCreator.h>

namespace {

class NoOpInvocationViewModelCallback final
    : public commonHead::viewModels::IInvocationViewModelCallback
{
};

} // namespace

TEST_CASE("InvocationViewModel creator returns its public API", "[InvocationViewModel][Api]")
{
    std::shared_ptr<commonHead::viewModels::IInvocationViewModel> viewModel =
        commonHead::viewModels::impl::createInvocationViewModel(
            commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "InvocationViewModel");
}

TEST_CASE("InvocationViewModel public API is safe without a framework", "[InvocationViewModel][Api]")
{
    auto viewModel = commonHead::viewModels::impl::createInvocationViewModel(
        commonHead::ICommonHeadFrameworkWptr{});
    auto callback = std::make_shared<NoOpInvocationViewModelCallback>();

    REQUIRE_NOTHROW(viewModel->registerCallback(callback));
    REQUIRE_NOTHROW(viewModel->initViewModel());
    REQUIRE_NOTHROW(viewModel->processStartupParameters());
    REQUIRE(viewModel->getStartupParameters().empty());
    REQUIRE_NOTHROW(viewModel->processCommandMessage("test-command"));
    REQUIRE_NOTHROW(viewModel->unRegisterCallback(callback));
}
