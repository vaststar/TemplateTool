#include <map>
#include <mutex>

#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/IDatabaseWrapper.h>
#include <ucf/Services/ServiceCommonFile/ServiceLogger.h>

#include "DataWarehouseService.h"
#include "DataWarehouseSchemas.h"


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
    void initializeDB(const model::DBConfig& dbConfig);
private:
    ucf::utilities::database::DataBaseSchemas createUserTables();
private:
    std::map<model::DBEnum, std::shared_ptr<ucf::utilities::database::IDatabaseWrapper>> mDataBaseWrapper;
};

DataWarehouseService::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
{

}

DataWarehouseService::DataPrivate::~DataPrivate()
{
    mDataBaseWrapper.clear();
}

void DataWarehouseService::DataPrivate::initializeDB(const model::DBConfig& dbConfig)
{
    if (mDataBaseWrapper.find(dbConfig.dbType) == mDataBaseWrapper.end())
    {
        mDataBaseWrapper[dbConfig.dbType] = ucf::utilities::database::IDatabaseWrapper::createSqliteDatabase(ucf::utilities::database::SqliteDatabaseConfig{dbConfig.dbFilePath, dbConfig.password});
        mDataBaseWrapper[dbConfig.dbType]->open();
        mDataBaseWrapper[dbConfig.dbType]->createTables(createUserTables());
    }
    else
    {
        SERVICE_LOG_WARN("already have dbType:" << static_cast<int>(dbConfig.dbType));
    }
}

ucf::utilities::database::DataBaseSchemas DataWarehouseService::DataPrivate::createUserTables()
{
    return {db::schema::UserContactTable{}, db::schema::GroupContactTable{}};
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

}

DataWarehouseService::~DataWarehouseService()
{

}

std::string DataWarehouseService::getServiceName() const
{
    return "DataWarehouseService";
}

void DataWarehouseService::initService()
{
    initializeDB({model::DBEnum::SHARED_DB, "shared_db.db"});
}

void DataWarehouseService::initializeDB(const model::DBConfig& dbConfig)
{
    mDataPrivate->initializeDB(dbConfig);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataWarehouseService Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}
