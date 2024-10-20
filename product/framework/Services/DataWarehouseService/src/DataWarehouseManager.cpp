#include <map>
#include <mutex>
#include <algorithm>
#include <functional>
#include <filesystem>

#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseValueStruct.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseSchema.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseDataRecord.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/IDatabaseWrapper.h>

#include <ucf/Services/DataWarehouseService/DatabaseModel.h>
#include <ucf/Services/DataWarehouseService/DataBaseDataValue.h>
#include <ucf/Services/DataWarehouseService/DataBaseTableModel.h>
#include <ucf/Services/ServiceCommonFile/ServiceLogger.h>

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
    DataPrivate();
    ~DataPrivate();
    void initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables);
    void insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values, const std::source_location location);
private:
    ucf::utilities::database::DatabaseSchemas convertToDatabaseSchemas(const std::vector<model::DBTableModel>& tableModels) const;
    ucf::utilities::database::ListOfArguments convertToDatabaseArguments(const model::ListOfDBValues& values) const;
    ucf::utilities::database::DatabaseValueStruct convertToDatabaseDataStruct(const model::DataBaseDataValue& value) const;
    void createFolder(const std::string& dbFilePath) const;
private:
    mutable std::mutex mDatabaseMutex;
    std::map<std::string, std::shared_ptr<ucf::utilities::database::IDatabaseWrapper>> mDatabaseWrapper;
};

DataWarehouseManager::DataPrivate::DataPrivate()
{
}

DataWarehouseManager::DataPrivate::~DataPrivate()
{
    mDatabaseWrapper.clear();
}

void DataWarehouseManager::DataPrivate::createFolder(const std::string& dbFilePath) const
{
    std::string dbFolder;
    if (size_t split_pos = dbFilePath.find_last_of("/"); split_pos != std::string::npos)
    {
        dbFolder = dbFilePath.substr(0, split_pos);
    }
    else if (size_t split_pos = dbFilePath.find_last_of("\\"); split_pos != std::string::npos)
    {
        dbFolder = dbFilePath.substr(0, split_pos);
    }
    if (!dbFolder.empty())
    {
        std::filesystem::create_directories(dbFolder);
    }
}

void DataWarehouseManager::DataPrivate::initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables)
{
    std::scoped_lock<std::mutex> loc(mDatabaseMutex);
    if (mDatabaseWrapper.find(dbConfig->getDBId()) == mDatabaseWrapper.end())
    {
        if (auto sqliteConfig = std::dynamic_pointer_cast<model::SqliteDBConfig>(dbConfig))
        {
            createFolder(sqliteConfig->getDBFilePath());
            auto dataBaseWrapper = ucf::utilities::database::IDatabaseWrapper::createSqliteDatabase(ucf::utilities::database::SqliteDatabaseConfig{sqliteConfig->getDBFilePath(), sqliteConfig->getDBPassword()});
            mDatabaseWrapper[dbConfig->getDBId()] = dataBaseWrapper;
            dataBaseWrapper->open();
            if (!tables.empty())
            {
                dataBaseWrapper->createTables(convertToDatabaseSchemas(tables));
            }
        }
    }
    else
    {
        SERVICE_LOG_WARN("already have db:" << dbConfig->getDBId());
    }
}

ucf::utilities::database::DatabaseSchemas DataWarehouseManager::DataPrivate::convertToDatabaseSchemas(const std::vector<model::DBTableModel>& tableModels) const
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

ucf::utilities::database::DatabaseValueStruct DataWarehouseManager::DataPrivate::convertToDatabaseDataStruct(const model::DataBaseDataValue& value) const
{
    if (value.holdsType<std::string>())
    {
        return ucf::utilities::database::DatabaseValueStruct(value.getStringValue());
    }
    else if (value.holdsType<int>())
    {
        return ucf::utilities::database::DatabaseValueStruct(value.getIntValue());
    }
    else if (value.holdsType<float>())
    {
        return ucf::utilities::database::DatabaseValueStruct(value.getFloatValue());
    }
    else if (value.holdsType<std::vector<uint8_t>>())
    {
        return ucf::utilities::database::DatabaseValueStruct(value.getBufferValue());
    }
    else
    {
        SERVICE_LOG_WARN("convert data failed");
        return ucf::utilities::database::DatabaseValueStruct("");
    }
}

ucf::utilities::database::ListOfArguments DataWarehouseManager::DataPrivate::convertToDatabaseArguments(const model::ListOfDBValues& values) const
{
    ucf::utilities::database::ListOfArguments listOfArguments;
    std::transform(values.cbegin(), values.cend(), std::back_inserter(listOfArguments), [this](const auto& value) {
        ucf::utilities::database::Arguments arguments; 
        std::transform(value.begin(), value.end(), std::back_inserter(arguments), std::bind_front(&DataWarehouseManager::DataPrivate::convertToDatabaseDataStruct, this));
        return arguments;
    });
    return listOfArguments;
}

void DataWarehouseManager::DataPrivate::insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values, const std::source_location location)
{
    std::scoped_lock<std::mutex> loc(mDatabaseMutex);
    if (auto it = mDatabaseWrapper.find(dbId); it != mDatabaseWrapper.end())
    {
        it->second->insertIntoDatabase(tableName, columnFields, convertToDatabaseArguments(values), location);

        it->second->fetchFromDatabase(tableName,{},[](const std::vector<ucf::utilities::database::DatabaseDataRecord>& results){
            //for (const auto& res: results)
            //{
            //    auto val = res.values();
            //}
            SERVICE_LOG_WARN("unrecognized db value type, will just use empty string");
        });
    }
    else
    {
        SERVICE_LOG_WARN("no db:" << dbId);
    }
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
DataWarehouseManager::DataWarehouseManager()
    : mDataPrivate(std::make_unique<DataWarehouseManager::DataPrivate>())
{

}

DataWarehouseManager::~DataWarehouseManager()
{

}

void DataWarehouseManager::initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables)
{
    mDataPrivate->initializeDB(dbConfig, tables);
}

void DataWarehouseManager::insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values, const std::source_location location)
{
    for (const auto& value: values)
    {
        if (columnFields.size() != value.size())
        {
            SERVICE_LOG_WARN("insert data but column fields size not matched, dbId" << dbId);
            return;
        }
    }

    mDataPrivate->insertIntoDatabase(dbId, tableName, columnFields, values, location);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataWarehouseManager Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}