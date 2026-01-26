#pragma once

#include <memory>
#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/CoreFramework/CoreFrameworkCallbackDefault.h>

namespace ucf::service{
class DataWarehouseService final: public IDataWarehouseService,
                                  public ucf::framework::CoreFrameworkCallbackDefault,
                                  public virtual ucf::utilities::NotificationHelper<IDataWarehouseServiceCallback>,
                                  public std::enable_shared_from_this<DataWarehouseService>
{
public:
    explicit DataWarehouseService(ucf::framework::ICoreFrameworkWPtr coreFramework);
    virtual ~DataWarehouseService();
    DataWarehouseService(const DataWarehouseService&) = delete;
    DataWarehouseService(DataWarehouseService&&) = delete;
    DataWarehouseService& operator=(const DataWarehouseService&) = delete;
    DataWarehouseService& operator=(DataWarehouseService&&) = delete;
public:
    //IService
    virtual std::string getServiceName() const override;

    //IDataWarehouseService
    virtual void initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables) override;
    virtual bool insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values, const std::source_location location = std::source_location::current()) override;
    virtual void fetchFromDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListsOfWhereCondition& whereConditions, model::DatabaseDataRecordsCallback func, int limit = 0, const std::source_location location = std::source_location::current()) override;
    virtual int64_t updateInDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::DBDataValues& values, const model::ListsOfWhereCondition& whereConditions, const std::source_location location = std::source_location::current()) override;
    virtual bool updateBatch(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& keyColumns, const model::DBColumnFields& valueColumns, const model::ListOfDBValues& items, const std::source_location location = std::source_location::current()) override;
    virtual int64_t deleteFromDatabase(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions, const std::source_location location = std::source_location::current()) override;
    virtual bool exists(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions) override;
    virtual int64_t count(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions) override;
    
    //ICoreFrameworkCallback
    virtual void onServiceInitialized() override;
    virtual void onCoreFrameworkExit() override;
protected:
    virtual void initService() override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}