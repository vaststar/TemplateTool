#pragma once

#include <memory>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/CoreFramework/CoreFrameworkCallbackDefault.h>

namespace ucf::service{
class DataWarehouseService final: public IDataWarehouseService,
                                  public ucf::framework::CoreFrameworkCallbackDefault,
                                  public std::enable_shared_from_this<DataWarehouseService>
{
public:
    explicit DataWarehouseService(ucf::framework::ICoreFrameworkWPtr coreFramework);
    virtual ~DataWarehouseService();
    DataWarehouseService(const DataWarehouseService&) = delete;
    DataWarehouseService(DataWarehouseService&&) = delete;
    DataWarehouseService& operator=(const DataWarehouseService&) = delete;
    DataWarehouseService& operator=(DataWarehouseService&&) = delete;

    //IService
    virtual std::string getServiceName() const override;

    //IDataWarehouseService
    virtual void initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables) override;
    virtual void insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values, const std::source_location location = std::source_location::current()) override;
    virtual void fetchFromDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListsOfWhereCondition& whereConditions, model::DatabaseDataRecordsCallback func, int limit = 0, const std::source_location location = std::source_location::current()) override;
    
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