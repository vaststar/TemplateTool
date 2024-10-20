#pragma once

#include <string>
#include <memory>
#include <vector>
#include <source_location>

#include <ucf/CoreFramework/IService.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{
    namespace model {
        class DBTableModel;
        class DataBaseDataValue;
        using DBDataValues = std::vector<DataBaseDataValue>;
        using ListOfDBValues = std::vector<DBDataValues>;    

        using DBColumnFields = std::vector<std::string>;
        class DBConfig;
    }
    

class SERVICE_EXPORT IDataWarehouseService: public IService
{
public:
    virtual void initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables) = 0;
    virtual void insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values, const std::source_location location = std::source_location::current()) = 0;
    // virtual void fetchFromDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListsOfWhereCondition& whereConditions, int limit = 0, const std::source_location location = std::source_location::current());
    static std::shared_ptr<IDataWarehouseService> CreateInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}