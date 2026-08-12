#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>

#include <commonhead/viewmodels/UpgradeViewModel/UpgradeViewModelCreator.h>

namespace {

class NoOpUpgradeViewModelCallback final
    : public commonHead::viewModels::IUpgradeViewModelCallback
{
public:
    void onUpgradeStateChanged(commonHead::viewModels::model::UpgradeViewState) override {}

    void onCheckCompleted(
        bool,
        const commonHead::viewModels::model::UpgradeViewInfo&) override {}

    void onDownloadProgress(int64_t, int64_t) override {}

    void onUpgradeError(const std::string&) override {}
};

} // namespace

TEST_CASE("UpgradeViewModel creator returns its public API", "[UpgradeViewModel][Api]")
{
    std::shared_ptr<commonHead::viewModels::IUpgradeViewModel> viewModel =
        commonHead::viewModels::impl::createUpgradeViewModel(
            commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "UpgradeViewModel");
}

TEST_CASE("UpgradeViewModel public API is safe without a framework", "[UpgradeViewModel][Api]")
{
    auto viewModel = commonHead::viewModels::impl::createUpgradeViewModel(
        commonHead::ICommonHeadFrameworkWptr{});
    auto callback = std::make_shared<NoOpUpgradeViewModelCallback>();

    REQUIRE_NOTHROW(viewModel->registerCallback(callback));
    REQUIRE_NOTHROW(viewModel->initViewModel());
    REQUIRE_NOTHROW(viewModel->checkForUpgrade());
    REQUIRE_NOTHROW(viewModel->downloadUpgrade());
    REQUIRE_NOTHROW(viewModel->installAndRestart());
    REQUIRE_NOTHROW(viewModel->cancelDownload());
    REQUIRE_NOTHROW(viewModel->dismissUpgrade());
    REQUIRE_NOTHROW(viewModel->unRegisterCallback(callback));
}

TEST_CASE("UpgradeViewModel API model has stable defaults", "[UpgradeViewModel][Api]")
{
    commonHead::viewModels::model::UpgradeViewInfo info;

    REQUIRE(info.version.empty());
    REQUIRE(info.releaseDate.empty());
    REQUIRE(info.releaseNotes.empty());
    REQUIRE_FALSE(info.mandatory);
}
