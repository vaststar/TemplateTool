#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>

#include <commonhead/viewmodels/MainWindowViewModel/MainWindowViewModelCreator.h>

namespace {

class CapturingMainWindowViewModelCallback final
    : public commonHead::viewModels::IMainWindowViewModelCallback
{
public:
    void onActivateMainWindow() override
    {
        activated = true;
    }

    void onLogsPackComplete(bool result, const std::string& path) override
    {
        logsPackCompleted = true;
        success = result;
        archivePath = path;
    }

    bool activated{false};
    bool logsPackCompleted{false};
    bool success{true};
    std::string archivePath{"unexpected"};
};

} // namespace

TEST_CASE("MainWindowViewModel creator returns its public API", "[MainWindowViewModel][Api]")
{
    std::shared_ptr<commonHead::viewModels::IMainWindowViewModel> viewModel =
        commonHead::viewModels::impl::createMainWindowViewModel(
            commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "MainWindowViewModel");
}

TEST_CASE("MainWindowViewModel reports log packing failure without a framework", "[MainWindowViewModel][Api]")
{
    auto viewModel = commonHead::viewModels::impl::createMainWindowViewModel(
        commonHead::ICommonHeadFrameworkWptr{});
    auto callback = std::make_shared<CapturingMainWindowViewModelCallback>();

    REQUIRE_NOTHROW(viewModel->registerCallback(callback));
    REQUIRE_NOTHROW(viewModel->initViewModel());
    REQUIRE_NOTHROW(viewModel->packApplicationLogs());
    REQUIRE(callback->logsPackCompleted);
    REQUIRE_FALSE(callback->success);
    REQUIRE(callback->archivePath.empty());
    REQUIRE_NOTHROW(viewModel->unRegisterCallback(callback));
}
