#include <catch2/catch_test_macros.hpp>

#include <memory>

#include <commonHead/viewModels/ClientInfoViewModel/ClientInfoModel.h>
#include <commonHead/viewModels/ClientInfoViewModel/ClientInfoViewModelCreator.h>

namespace {

class NoOpClientInfoViewModelCallback final
    : public commonHead::viewModels::IClientInfoViewModelCallback
{
};

} // namespace

TEST_CASE("ClientInfoViewModel creator returns its public API", "[ClientInfoViewModel][Api]")
{
    std::shared_ptr<commonHead::viewModels::IClientInfoViewModel> viewModel =
        commonHead::viewModels::impl::createClientInfoViewModel(
            commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "ClientInfoViewModel");
}

TEST_CASE("ClientInfoViewModel public API has safe defaults without a framework", "[ClientInfoViewModel][Api]")
{
    using commonHead::viewModels::model::LanguageType;
    using commonHead::viewModels::model::ThemeType;

    auto viewModel = commonHead::viewModels::impl::createClientInfoViewModel(
        commonHead::ICommonHeadFrameworkWptr{});
    auto callback = std::make_shared<NoOpClientInfoViewModelCallback>();

    REQUIRE_NOTHROW(viewModel->registerCallback(callback));
    REQUIRE_NOTHROW(viewModel->initViewModel());
    REQUIRE(viewModel->getApplicationVersion() == "0.0.0");
    REQUIRE(viewModel->getProductName().empty());
    REQUIRE(viewModel->getCompanyName().empty());
    REQUIRE(viewModel->getCopyright().empty());
    REQUIRE(viewModel->getProductDescription().empty());
    REQUIRE(viewModel->getApplicationLanguage() == LanguageType::ENGLISH);
    REQUIRE(viewModel->getSupportedLanguages().empty());
    REQUIRE_NOTHROW(viewModel->setApplicationLanguage(LanguageType::CHINESE_SIMPLIFIED));
    REQUIRE(viewModel->getCurrentThemeType() == ThemeType::SystemDefault);
    REQUIRE(viewModel->getSupportedThemeTypes().empty());
    REQUIRE_NOTHROW(viewModel->setCurrentThemeType(ThemeType::Dark));
    REQUIRE_NOTHROW(viewModel->unRegisterCallback(callback));
}
