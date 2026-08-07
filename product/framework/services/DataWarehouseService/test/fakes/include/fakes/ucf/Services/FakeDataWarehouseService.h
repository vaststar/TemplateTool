#pragma once

#include <source_location>

#include <trompeloeil.hpp>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/Services/DataWarehouseService/DatabaseDataValue.h>
#include <ucf/Services/DataWarehouseService/DatabaseConfig.h>
#include <ucf/Services/DataWarehouseService/DatabaseTableModel.h>
#include <ucf/Services/DataWarehouseService/DatabaseDataRecord.h>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>

namespace ucf::service::fakes {

/**
 * @brief Fake implementation of IDataWarehouseService for unit testing
 *
 * Usage:
 *   auto fakeService = std::make_shared<FakeDataWarehouseService>();
 *   ALLOW_CALL(*fakeService, isDatabaseReady(trompeloeil::_)).RETURN(true);
 *
 * Note: methods that carry a trailing std::source_location are mocked without the
 * default argument; call them through the IDataWarehouseService interface so the
 * base-class default applies, or pass an explicit std::source_location.
 */
class FakeDataWarehouseService
    : public IDataWarehouseService
    , public ucf::utilities::NotificationHelper<IDataWarehouseServiceCallback>
{
public:
    FakeDataWarehouseService() = default;
    ~FakeDataWarehouseService() override = default;

    // IService
    MAKE_CONST_MOCK0(getServiceName, std::string(), override);

    // IDataWarehouseService
    MAKE_MOCK2(initializeDB,
        InitializeDBResult(std::shared_ptr<model::DBConfig>, const std::vector<model::DBTableModel>&), override);
    MAKE_CONST_MOCK1(isDatabaseReady, bool(const std::string&), override);

    MAKE_MOCK5(insertIntoDatabase,
        bool(const std::string&, const std::string&, const model::DBColumnFields&,
             const model::ListOfDBValues&, std::source_location), override);
    MAKE_MOCK7(fetchFromDatabase,
        void(const std::string&, const std::string&, const model::DBColumnFields&,
             const model::ListsOfWhereCondition&, model::DatabaseDataRecordsCallback,
             int, std::source_location), override);
    MAKE_MOCK6(updateInDatabase,
        int64_t(const std::string&, const std::string&, const model::DBColumnFields&,
                const model::DBDataValues&, const model::ListsOfWhereCondition&, std::source_location), override);
    MAKE_MOCK6(updateBatch,
        bool(const std::string&, const std::string&, const model::DBColumnFields&,
             const model::DBColumnFields&, const model::ListOfDBValues&, std::source_location), override);
    MAKE_MOCK4(deleteFromDatabase,
        int64_t(const std::string&, const std::string&, const model::ListsOfWhereCondition&, std::source_location), override);
    MAKE_MOCK3(exists,
        bool(const std::string&, const std::string&, const model::ListsOfWhereCondition&), override);
    MAKE_MOCK3(count,
        int64_t(const std::string&, const std::string&, const model::ListsOfWhereCondition&), override);
    MAKE_MOCK2(atomicWrite,
        bool(const std::string&, std::function<bool()>), override);

protected:
    void initService() override {}
    void deinitService() override {}
};

} // namespace ucf::service::fakes
