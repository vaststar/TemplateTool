#include <catch2/catch_test_macros.hpp>

#include <commonhead/viewmodels/Base64ToolViewModel/Base64ToolViewModelCreator.h>

TEST_CASE("Base64ToolViewModel encodes and decodes text", "[Base64ToolViewModel]")
{
    auto viewModel = commonHead::viewModels::impl::createBase64ToolViewModel(
        commonHead::ICommonHeadFrameworkWptr{});

    REQUIRE(viewModel != nullptr);
    REQUIRE(viewModel->getViewModelName() == "Base64ToolViewModel");
    REQUIRE_NOTHROW(viewModel->initViewModel());

    const auto encoded = viewModel->encode("hello");
    REQUIRE(encoded.success);
    REQUIRE(encoded.data == "aGVsbG8=");

    const auto decoded = viewModel->decode(encoded.data);
    REQUIRE(decoded.success);
    REQUIRE(decoded.data == "hello");

    const auto invalid = viewModel->decode("***");
    REQUIRE_FALSE(invalid.success);
    REQUIRE_FALSE(invalid.errorMessage.empty());
}
