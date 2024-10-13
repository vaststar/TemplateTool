#include <map>
#include <mutex>
#include <algorithm>

#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseFormatStruct.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/IDatabaseWrapper.h>

#include <ucf/Services/DataWarehouseService/DatabaseModel.h>
#include <ucf/Services/ServiceCommonFile/ServiceLogger.h>

//#include "DataWarehouseSchemas.h"
#include "DataWarehouseManager.h"

namespace ucf::service{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class DataWarehouseManager::DataPrivate
{
public:
    explicit DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~DataPrivate();
    void initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables);
private:
    ucf::utilities::database::DatabaseSchemas convertToDatabaseSchemas(const std::vector<model::DBTableModel>& tableModels);

private:
    mutable std::mutex mDatabaseMutex;
    std::map<std::string, std::shared_ptr<ucf::utilities::database::IDatabaseWrapper>> mDatabaseWrapper;
};

DataWarehouseManager::DataPrivate::DataPrivate(ucf::framework::ICoreFrameworkWPtr coreFramework)
{

}

DataWarehouseManager::DataPrivate::~DataPrivate()
{
    mDatabaseWrapper.clear();
}

void DataWarehouseManager::DataPrivate::initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables)
{
    if (mDatabaseWrapper.find(dbConfig->getDBId()) == mDatabaseWrapper.end())
    {
        if (auto sqliteConfig = std::dynamic_pointer_cast<model::SqliteDBConfig>(dbConfig))
        {
            auto dataBaseWrapper = ucf::utilities::database::IDatabaseWrapper::createSqliteDatabase(ucf::utilities::database::SqliteDatabaseConfig{sqliteConfig->getDBFilePath(), sqliteConfig->getDBPassword()});
            mDatabaseWrapper[dbConfig->getDBId()] = dataBaseWrapper;
            dataBaseWrapper->open();
            if (!tables.empty())
            {
                dataBaseWrapper->createTables(convertToDatabaseSchemas(tables));
            }
        }

        ucf::utilities::database::ListOfArguments arguments;
        arguments.emplace_back(ucf::utilities::database::Arguments{ std::string("test_id"), std::string("test_nameee"), std::string("243@qq.com") });
        arguments.emplace_back(ucf::utilities::database::Arguments{ std::string("test_id2"), std::string("test_name2"), std::string("111@qq.com") });
        arguments.emplace_back(ucf::utilities::database::Arguments{ std::string("test_id5"), std::string("test5_name4"), std::string("qqq@qq.com") });
        mDatabaseWrapper[dbConfig->getDBId()]->insertIntoDatabase("UserContact",
            {"CONTACT_ID", "CONTACT_FULL_NAME", "CONTACT_EMAIL"},
            arguments);
    }
    else
    {
        SERVICE_LOG_WARN("already have db:" << dbConfig->getDBId());
    }
}

ucf::utilities::database::DatabaseSchemas DataWarehouseManager::DataPrivate::convertToDatabaseSchemas(const std::vector<model::DBTableModel>& tableModels)
{
    ucf::utilities::database::DatabaseSchemas databaseSchemas;
    std::transform(tableModels.cbegin(), tableModels.cend(), std::back_inserter(databaseSchemas), [](const auto& table) {
        std::vector<ucf::utilities::database::DatabaseSchema::Column> columns;
        auto tableColumns = table.columns();
        std::transform(tableColumns.cbegin(), tableColumns.cend(), std::back_inserter(columns), [](const auto& column) {
            return ucf::utilities::database::DatabaseSchema::Column{column.mName, column.mAttributes};
        });
        return ucf::utilities::database::DatabaseSchema(table.tableName(), columns);
    });
    return databaseSchemas;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataWarehouseManager Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
DataWarehouseManager::DataWarehouseManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mDataPrivate(std::make_unique<DataWarehouseManager::DataPrivate>(coreFramework))
{

}

DataWarehouseManager::~DataWarehouseManager()
{

}
void DataWarehouseManager::initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables)
{
    mDataPrivate->initializeDB(dbConfig, tables);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataWarehouseManager Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}