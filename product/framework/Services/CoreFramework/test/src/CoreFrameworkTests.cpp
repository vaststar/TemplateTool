#include <catch2/catch_test_macros.hpp>
#include <ucf/CoreFramework/ICoreFramework.h>

// #include <fakes/FakeCoreFramework.h>

#include "CoreFramework.h"

TEST_CASE( "Basic Test for CoreFramework", "[CoreFrameworkTests]" )
{
    auto coreFramework = ucf::framework::ICoreFramework::createInstance();
    REQUIRE(coreFramework != nullptr);

    // auto fakeCoreFramework = std::make_shared<fakes::FakeCoreFramework>();
    // REQUIRE(fakeCoreFramework != nullptr);

    auto coreFrameworkPtr = std::make_shared<ucf::framework::CoreFramework>();
    REQUIRE(coreFrameworkPtr != nullptr);
}