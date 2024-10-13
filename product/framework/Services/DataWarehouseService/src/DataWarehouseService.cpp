#include <map>
#include <mutex>

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/IDatabaseWrapper.h>
#include <ucf/Services/ServiceCommonFile/ServiceLogger.h>
#include <ucf/Services/DataWarehouseService/DatabaseModel.h>

#include "DataWarehouseService.h"
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
    void initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables);
    void insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values);
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

void DataWarehouseService::DataPrivate::initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables)
{
    mDataWarehouseManager->initializeDB(dbConfig, tables);
}

void DataWarehouseService::DataPrivate::insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values)
{
    mDataWarehouseManager->insertIntoDatabase(dbId, tableName, columnFields, values);
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
std::shared_ptr<IDataWarehouseService> IDataWarehouseService::CreateInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
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

void DataWarehouseService::OnServiceInitialized()
{
    SERVICE_LOG_DEBUG("");
}

void DataWarehouseService::onCoreFrameworkExit()
{
    SERVICE_LOG_DEBUG("");
}

void DataWarehouseService::initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables)
{
    mDataPrivate->initializeDB(dbConfig, tables);
}

void DataWarehouseService::insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values)
{
    mDataPrivate->insertIntoDatabase(dbId, tableName, columnFields, values);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataWarehouseService Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}
