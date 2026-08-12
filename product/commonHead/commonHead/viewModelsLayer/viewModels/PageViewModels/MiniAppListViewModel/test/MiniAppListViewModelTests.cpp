#include <catch2/catch_test_macros.hpp>

#include <commonHead/viewModels/MiniAppListViewModel/MiniAppListViewModelCreator.h>

TEST_CASE("MiniAppListViewModel creator returns its public API",
          "[MiniAppListViewModel][Api]")
{
    using namespace commonHead::viewModels;

    auto viewModel = impl::createMiniAppListViewModel(commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "MiniAppListViewModel");
    REQUIRE(viewModel->getMiniApps().empty());

    const auto missing = viewModel->getMiniApp("missing");
    REQUIRE(missing.id.empty());
    REQUIRE(missing.name.empty());
    REQUIRE(missing.permissions.empty());

    viewModel->installMiniApp("missing");
    viewModel->uninstallMiniApp("missing");
    REQUIRE(viewModel->getMiniApps().empty());
}

TEST_CASE("MiniAppListViewModel exposes standalone mini-app types",
          "[MiniAppListViewModel][Types]")
{
    using commonHead::viewModels::model::MiniAppInfo;

    MiniAppInfo info;
    info.id = "app";
    info.name = "Mini App";
    info.description = "Description";
    info.entry = "index.html";
    info.iconPath = "/icon.png";
    info.packageDir = "/package";
    info.storageDir = "/storage";
    info.cacheDir = "/cache";
    info.permissions = {"storage.read", "storage.write"};

    REQUIRE(info.id == "app");
    REQUIRE(info.name == "Mini App");
    REQUIRE(info.description == "Description");
    REQUIRE(info.entry == "index.html");
    REQUIRE(info.iconPath == "/icon.png");
    REQUIRE(info.packageDir == "/package");
    REQUIRE(info.storageDir == "/storage");
    REQUIRE(info.cacheDir == "/cache");
    REQUIRE(info.permissions.size() == 2);
}
