#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/Services/FakeServiceFactory.h>

using namespace ucf::service;

TEST_CASE("FakeServiceFactory can be constructed", "[ServiceFactory]")
{
    auto fakeFactory = std::make_shared<fakes::FakeServiceFactory>();
    REQUIRE(fakeFactory != nullptr);
}
