#include <catch2/catch_test_macros.hpp>

#include <commonhead/viewmodels/NetworkProxyViewModel/NetworkProxyViewModelCreator.h>

TEST_CASE("NetworkProxyViewModel creator returns its public API",
          "[NetworkProxyViewModel][Api]")
{
    using namespace commonHead::viewModels;

    auto viewModel = impl::createNetworkProxyViewModel(commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "NetworkProxyViewModel");
    REQUIRE(viewModel->proxyState() == model::ProxyState::Idle);
    REQUIRE(viewModel->certStatus() == model::CertStatus::Unknown);
    REQUIRE(viewModel->caCertPath().empty());

    viewModel->sendCommand("{}");
    viewModel->setInterceptEnabled(false);
    viewModel->resumeRequest("missing");
    viewModel->dropRequest("missing");
    viewModel->updateRules("mock", "[]");
    viewModel->checkCertStatus();
    viewModel->stopProxy();

    REQUIRE(viewModel->proxyState() == model::ProxyState::Idle);
}

TEST_CASE("NetworkProxyViewModel exposes standalone proxy types",
          "[NetworkProxyViewModel][Types]")
{
    using namespace commonHead::viewModels::model;

    ProxyConfig config;
    REQUIRE(config.proxyPort == 8080);
    REQUIRE(config.autoSystemProxy);
    REQUIRE(config.stopTimeoutMs == 3000);

    REQUIRE(ProxyState::Idle != ProxyState::Running);
    REQUIRE(CertStatus::Unknown != CertStatus::Trusted);
    REQUIRE(CertInstallResult::Success != CertInstallResult::Failed);
}
