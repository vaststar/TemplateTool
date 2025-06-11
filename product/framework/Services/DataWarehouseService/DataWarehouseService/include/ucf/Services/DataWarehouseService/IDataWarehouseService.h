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
class SERVICE_EXPORT IDataWarehouseService: public IService, public virtual ucf::utilities::INotificationHelper<IDataWarehouseServiceCallback>
{
public:
    virtual void initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables) = 0;
    virtual void insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values, const std::source_location location = std::source_location::current()) = 0;
    virtual void fetchFromDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListsOfWhereCondition& whereConditions, model::DatabaseDataRecordsCallback func, int limit = 0, const std::source_location location = std::source_location::current()) = 0;
    static std::shared_ptr<IDataWarehouseService> createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}