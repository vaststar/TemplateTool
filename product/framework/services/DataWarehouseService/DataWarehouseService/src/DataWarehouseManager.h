#pragma once

#include <memory>
#include <source_location>
#include <ucf/Services/DataWarehouseService/DatabaseModelDeclare.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{
class DataWarehouseManager final
{
public:
    DataWarehouseManager();
    ~DataWarehouseManager();
    DataWarehouseManager(const DataWarehouseManager&) = delete;
    DataWarehouseManager(DataWarehouseManager&&) = delete;
    DataWarehouseManager& operator=(const DataWarehouseManager&) = delete;
    DataWarehouseManager& operator=(DataWarehouseManager&&) = delete;
public:
    void initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables);
    bool insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values, const std::source_location location);
    void fetchFromDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListsOfWhereCondition& whereConditions, model::DatabaseDataRecordsCallback func, int limit, const std::source_location location);
    int64_t updateInDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::DBDataValues& values, const model::ListsOfWhereCondition& whereConditions, const std::source_location location);
    bool updateBatch(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& keyColumns, const model::DBColumnFields& valueColumns, const model::ListOfDBValues& items, const std::source_location location);
    int64_t deleteFromDatabase(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions, const std::source_location location);
    bool exists(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions);
    int64_t count(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions);
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}