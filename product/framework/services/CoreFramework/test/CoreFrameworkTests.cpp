#include <catch2/catch_test_macros.hpp>
#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/CoreFramework/CoreFrameworkCreator.h>

TEST_CASE("CoreFramework can be created through its public creator", "[CoreFramework]")
{
    auto coreFramework = ucf::framework::createCoreFramework();
    REQUIRE(coreFramework != nullptr);
    REQUIRE(coreFramework->getName() == "CoreFramework");
}
