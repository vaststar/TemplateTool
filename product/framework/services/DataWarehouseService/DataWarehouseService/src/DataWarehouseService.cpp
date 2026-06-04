#include "DataWarehouseService.h"

#include <map>
#include <mutex>

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Infrastructure/DatabaseClient/IDatabaseWrapper.h>
#include <ucf/Services/DataWarehouseService/DatabaseConfig.h>

#include "DataWarehouseServiceLogger.h"
#include "DataWarehouseManager.h"

namespace ucf::service{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class DataWarehouseService::DataPrivate
{
public:
    explicit DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~DataPrivate();
    ucf::framework::ICoreFrameworkWPtr getCoreFramework() const;
    InitializeDBResult initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables);
    bool isDatabaseReady(const std::string& dbId) const;
    bool insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values, const std::source_location location);
    void fetchFromDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListsOfWhereCondition& whereConditions, model::DatabaseDataRecordsCallback func, int limit, const std::source_location location);
    int64_t updateInDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::DBDataValues& values, const model::ListsOfWhereCondition& whereConditions, const std::source_location location);
    bool updateBatch(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& keyColumns, const model::DBColumnFields& valueColumns, const model::ListOfDBValues& items, const std::source_location location);
    int64_t deleteFromDatabase(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions, const std::source_location location);
    bool exists(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions);
    int64_t count(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions);
    bool atomicWrite(const std::string& dbId, std::function<bool()> work);
private:
    std::unique_ptr<DataWarehouseManager> mDataWarehouseManager;
    ucf::framework::ICoreFrameworkWPtr mCoreFramework;
};

DataWarehouseService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFramework(coreFramework)
    , mDataWarehouseManager(std::make_unique<DataWarehouseManager>())
{

}

DataWarehouseService::DataPrivate::~DataPrivate()
{
}

ucf::framework::ICoreFrameworkWPtr DataWarehouseService::DataPrivate::getCoreFramework() const
{
    return mCoreFramework;
}

InitializeDBResult DataWarehouseService::DataPrivate::initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables)
{
    return mDataWarehouseManager->initializeDB(dbConfig, tables);
}

bool DataWarehouseService::DataPrivate::isDatabaseReady(const std::string& dbId) const
{
    return mDataWarehouseManager->isDatabaseReady(dbId);
}

bool DataWarehouseService::DataPrivate::insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values, const std::source_location location)
{
    return mDataWarehouseManager->insertIntoDatabase(dbId, tableName, columnFields, values, location);
}

void DataWarehouseService::DataPrivate::fetchFromDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListsOfWhereCondition& whereConditions, model::DatabaseDataRecordsCallback func, int limit, const std::source_location location)
{
    mDataWarehouseManager->fetchFromDatabase(dbId, tableName, columnFields, whereConditions, func, limit, location);
}

int64_t DataWarehouseService::DataPrivate::updateInDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::DBDataValues& values, const model::ListsOfWhereCondition& whereConditions, const std::source_location location)
{
    return mDataWarehouseManager->updateInDatabase(dbId, tableName, columnFields, values, whereConditions, location);
}

bool DataWarehouseService::DataPrivate::updateBatch(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& keyColumns, const model::DBColumnFields& valueColumns, const model::ListOfDBValues& items, const std::source_location location)
{
    return mDataWarehouseManager->updateBatch(dbId, tableName, keyColumns, valueColumns, items, location);
}

int64_t DataWarehouseService::DataPrivate::deleteFromDatabase(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions, const std::source_location location)
{
    return mDataWarehouseManager->deleteFromDatabase(dbId, tableName, whereConditions, location);
}

bool DataWarehouseService::DataPrivate::exists(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions)
{
    return mDataWarehouseManager->exists(dbId, tableName, whereConditions);
}

int64_t DataWarehouseService::DataPrivate::count(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions)
{
    return mDataWarehouseManager->count(dbId, tableName, whereConditions);
}

bool DataWarehouseService::DataPrivate::atomicWrite(const std::string& dbId, std::function<bool()> work)
{
    return mDataWarehouseManager->atomicWrite(dbId, std::move(work));
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataWarehouseService Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<IDataWarehouseService> IDataWarehouseService::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<DataWarehouseService>(coreFramework);
}

DataWarehouseService::DataWarehouseService(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mDataPrivate(std::make_unique<DataWarehouseService::DataPrivate>(coreFramework))
{
    SERVICE_LOG_INFO("create DataWarehouseService");
}

DataWarehouseService::~DataWarehouseService()
{
    SERVICE_LOG_INFO("delete DataWarehouseService");
}

std::string DataWarehouseService::getServiceName() const
{
    return "DataWarehouseService";
}

void DataWarehouseService::initService()
{
    if (auto coreFramework = mDataPrivate->getCoreFramework().lock())
    {
        coreFramework->registerCallback(shared_from_this());
    }
}

void DataWarehouseService::onServiceInitialized()
{
    SERVICE_LOG_DEBUG("");
}

void DataWarehouseService::onCoreFrameworkExit()
{
    SERVICE_LOG_DEBUG("");
}

InitializeDBResult DataWarehouseService::initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables)
{
    // Reentrancy: Manager dedupes by dbId. Only fire OnDatabaseInitialized for the
    // first successful open so subscribers do not see a duplicate ready event on
    // repeat calls. Late subscribers should use isDatabaseReady() to short-circuit.
    const auto result = mDataPrivate->initializeDB(dbConfig, tables);
    switch (result)
    {
    case InitializeDBResult::Created:
        SERVICE_LOG_INFO("initializeDB created, dbId:" << dbConfig->getDBId() << ", firing OnDatabaseInitialized");
        fireNotification(&IDataWarehouseServiceCallback::OnDatabaseInitialized, dbConfig->getDBId());
        break;
    case InitializeDBResult::AlreadyExists:
        SERVICE_LOG_DEBUG("initializeDB no-op, already initialized, dbId:" << dbConfig->getDBId());
        break;
    case InitializeDBResult::Failed:
        SERVICE_LOG_ERROR("initializeDB failed, dbId:" << dbConfig->getDBId());
        break;
    }
    return result;
}

bool DataWarehouseService::isDatabaseReady(const std::string& dbId) const
{
    return mDataPrivate->isDatabaseReady(dbId);
}

bool DataWarehouseService::insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values, const std::source_location location)
{
    SERVICE_LOG_DEBUG("about to insert data into table: " << tableName << ", from: "
              << location.file_name() << '('
              << location.line() << ':'
              << location.column() << ") `"
              << location.function_name());
    return mDataPrivate->insertIntoDatabase(dbId, tableName, columnFields, values, location);
}

void DataWarehouseService::fetchFromDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListsOfWhereCondition& whereConditions, model::DatabaseDataRecordsCallback func, int limit, const std::source_location location)
{
    SERVICE_LOG_DEBUG("about to fetch data from table: " << tableName << ", from: "
              << location.file_name() << '('
              << location.line() << ':'
              << location.column() << ") `"
              << location.function_name());
    mDataPrivate->fetchFromDatabase(dbId, tableName, columnFields, whereConditions, func, limit, location);
}

int64_t DataWarehouseService::updateInDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::DBDataValues& values, const model::ListsOfWhereCondition& whereConditions, const std::source_location location)
{
    SERVICE_LOG_DEBUG("about to update data in table: " << tableName << ", from: "
              << location.file_name() << '('
              << location.line() << ':'
              << location.column() << ") `"
              << location.function_name());
    return mDataPrivate->updateInDatabase(dbId, tableName, columnFields, values, whereConditions, location);
}

bool DataWarehouseService::updateBatch(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& keyColumns, const model::DBColumnFields& valueColumns, const model::ListOfDBValues& items, const std::source_location location)
{
    SERVICE_LOG_DEBUG("about to batch update table: " << tableName << ", rows: " << items.size() << ", from: "
              << location.file_name() << '(' << location.line() << ')');
    return mDataPrivate->updateBatch(dbId, tableName, keyColumns, valueColumns, items, location);
}

int64_t DataWarehouseService::deleteFromDatabase(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions, const std::source_location location)
{
    SERVICE_LOG_DEBUG("about to delete from table: " << tableName << ", from: "
              << location.file_name() << '('
              << location.line() << ':'
              << location.column() << ") `"
              << location.function_name());
    return mDataPrivate->deleteFromDatabase(dbId, tableName, whereConditions, location);
}

bool DataWarehouseService::exists(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions)
{
    return mDataPrivate->exists(dbId, tableName, whereConditions);
}

int64_t DataWarehouseService::count(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions)
{
    return mDataPrivate->count(dbId, tableName, whereConditions);
}

bool DataWarehouseService::atomicWrite(const std::string& dbId, std::function<bool()> work)
{
    SERVICE_LOG_DEBUG("about to perform atomic write on dbId: " << dbId);
    return mDataPrivate->atomicWrite(dbId, std::move(work));
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataWarehouseService Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}
