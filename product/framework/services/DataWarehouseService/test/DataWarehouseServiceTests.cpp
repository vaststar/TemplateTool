#include <catch2/catch_test_macros.hpp>

#include <fakes/ucf/CoreFramework/FakeCoreFramework.h>
#include <ucf/services/DataWarehouseService/DatabaseConfig.h>
#include <ucf/services/DataWarehouseService/DatabaseTableModel.h>
#include <ucf/services/DataWarehouseService/DataWarehouseServiceCreator.h>
#include <ucf/services/DataWarehouseService/IDataWarehouseService.h>

TEST_CASE("DataWarehouseService can be created through its public creator", "[DataWarehouseService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    REQUIRE(fakeCoreFramework != nullptr);

    auto service = ucf::service::impl::createDataWarehouseService(fakeCoreFramework);
    REQUIRE(service != nullptr);
    REQUIRE(service->getServiceName() == "DataWarehouseService");
}

TEST_CASE("DataWarehouseService rejects unsupported database configuration", "[DataWarehouseService]")
{
    auto fakeCoreFramework = std::make_shared<ucf::framework::fakes::FakeCoreFramework>();
    auto service = ucf::service::impl::createDataWarehouseService(fakeCoreFramework);
    const ucf::service::model::DBConfig unsupportedConfig("unsupported");

    REQUIRE(
        service->initializeDB(unsupportedConfig, {}) ==
        ucf::service::InitializeDBResult::Failed);
}
