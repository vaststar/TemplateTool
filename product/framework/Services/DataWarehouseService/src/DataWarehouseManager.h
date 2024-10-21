#pragma once

#include <memory>
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
    void insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values, const std::source_location location);
    void fetchFromDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListsOfWhereCondition& whereConditions, model::DatabaseDataRecordsCallback func, int limit, const std::source_location location);
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}