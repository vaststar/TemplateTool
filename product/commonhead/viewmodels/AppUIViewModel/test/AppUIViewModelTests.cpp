#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>

#include <commonhead/viewmodels/AppUIViewModel/AppUIViewModelCreator.h>

namespace {

class RecordingAppUIViewModelCallback final
    : public commonHead::viewModels::IAppUIViewModelCallback
{
public:
    void onAppConfigInitialized() override { ++readyCount; }
    void onAppConfigInitializationFailed(const std::string& reason) override
    {
        ++failureCount;
        failureReason = reason;
    }

    int readyCount = 0;
    int failureCount = 0;
    std::string failureReason;
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

TEST_CASE("AppUIViewModel reports initialization failure without a framework", "[AppUIViewModel][Api]")
{
    auto viewModel = commonHead::viewModels::impl::createAppUIViewModel(
        commonHead::ICommonHeadFrameworkWptr{});
    auto callback = std::make_shared<RecordingAppUIViewModelCallback>();

    REQUIRE_NOTHROW(viewModel->registerCallback(callback));
    REQUIRE_NOTHROW(viewModel->initViewModel());
    REQUIRE(callback->readyCount == 0);
    REQUIRE(callback->failureCount == 1);
    REQUIRE(callback->failureReason.find("CommonHeadFramework") != std::string::npos);

    REQUIRE_NOTHROW(viewModel->initViewModel());
    REQUIRE(callback->readyCount == 0);
    REQUIRE(callback->failureCount == 1);
    REQUIRE_NOTHROW(viewModel->unRegisterCallback(callback));
}
