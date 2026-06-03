#pragma once

#include <string>
#include <memory>
#include <source_location>

#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>

#include <ucf/Services/ServiceDeclaration/IService.h>
#include <ucf/Services/DataWarehouseService/DatabaseModelDeclare.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseServiceCallback.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{

// Outcome of IDataWarehouseService::initializeDB. Lets callers distinguish first
// init (which fires OnDatabaseInitialized) from a no-op repeat or a failure.
enum class InitializeDBResult {
    Created,         // DB was opened and tables created on this call; notification fired.
    AlreadyExists,   // Same dbId was already open; nothing changed; notification NOT fired.
    Failed,          // open() failed; nothing was registered; notification NOT fired.
};

class SERVICE_EXPORT IDataWarehouseService: public IService, public virtual ucf::utilities::INotificationHelper<IDataWarehouseServiceCallback>
{
public:
    IDataWarehouseService() = default;
    IDataWarehouseService(const IDataWarehouseService&) = delete;
    IDataWarehouseService(IDataWarehouseService&&) = delete;
    IDataWarehouseService& operator=(const IDataWarehouseService&) = delete;
    IDataWarehouseService& operator=(IDataWarehouseService&&) = delete;
    virtual ~IDataWarehouseService() = default;
public:
    // Idempotent: a second call with the same dbId returns AlreadyExists and does
    // NOT re-fire OnDatabaseInitialized. Late subscribers must use isDatabaseReady()
    // to short-circuit on registration because the ready event fires exactly once.
    virtual InitializeDBResult initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables) = 0;

    // True iff initializeDB has previously succeeded for dbId and the DB is still open.
    [[nodiscard]] virtual bool isDatabaseReady(const std::string& dbId) const = 0;

    virtual bool insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values, const std::source_location location = std::source_location::current()) = 0;
    virtual void fetchFromDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListsOfWhereCondition& whereConditions, model::DatabaseDataRecordsCallback func, int limit = 0, const std::source_location location = std::source_location::current()) = 0;
    virtual int64_t updateInDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::DBDataValues& values, const model::ListsOfWhereCondition& whereConditions, const std::source_location location = std::source_location::current()) = 0;
    virtual bool updateBatch(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& keyColumns, const model::DBColumnFields& valueColumns, const model::ListOfDBValues& items, const std::source_location location = std::source_location::current()) = 0;
    virtual int64_t deleteFromDatabase(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions, const std::source_location location = std::source_location::current()) = 0;
    virtual bool exists(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions) = 0;
    virtual int64_t count(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions) = 0;
    static std::shared_ptr<IDataWarehouseService> createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}
