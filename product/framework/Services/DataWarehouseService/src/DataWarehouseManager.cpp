#include <map>
#include <mutex>
#include <algorithm>

#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseFormatStruct.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/IDatabaseWrapper.h>

#include <ucf/Services/DataWarehouseService/DatabaseModel.h>
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
    void insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values);
private:
    ucf::utilities::database::DatabaseSchemas convertToDatabaseSchemas(const std::vector<model::DBTableModel>& tableModels);
    ucf::utilities::database::ListOfArguments convertToDatabaseArguments(const model::ListOfDBValues& values);
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

void DataWarehouseManager::DataPrivate::initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables)
{
    std::scoped_lock<std::mutex> loc(mDatabaseMutex);
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


ucf::utilities::database::ListOfArguments DataWarehouseManager::DataPrivate::convertToDatabaseArguments(const model::ListOfDBValues& values)
{
    ucf::utilities::database::ListOfArguments listOfArguments;
    std::transform(values.cbegin(), values.cend(), std::back_inserter(listOfArguments), [](const auto& value) {
        ucf::utilities::database::Arguments arguments; 
        std::transform(value.begin(), value.end(), std::back_inserter(arguments), [](const model::DBDataValue& data) {
            if (const auto val = std::get_if<model::DBDataValueType::BOOL>(&data))
            {
                return ucf::utilities::database::DBFormatStruct(*val);
            }
            else if (const auto val = std::get_if<model::DBDataValueType::LONG>(&data))
            {
                return ucf::utilities::database::DBFormatStruct(*val);
            }
            else if (const auto val = std::get_if<model::DBDataValueType::LONGLONG>(&data))
            {
                return ucf::utilities::database::DBFormatStruct(*val);
            }
            else if (const auto val = std::get_if<model::DBDataValueType::STRING>(&data))
            {
                return ucf::utilities::database::DBFormatStruct(*val);
            }
            else if (const auto val = std::get_if<model::DBDataValueType::INT>(&data))
            {
                return ucf::utilities::database::DBFormatStruct(*val);
            }
            else if (const auto val = std::get_if<model::DBDataValueType::FLOAT>(&data))
            {
                return ucf::utilities::database::DBFormatStruct(*val);
            }
            else if (const auto val = std::get_if<model::DBDataValueType::BUFFER>(&data))
            {
                return ucf::utilities::database::DBFormatStruct(*val);
            }
            SERVICE_LOG_WARN("unrecognized db value type, will just use empty string");
            return ucf::utilities::database::DBFormatStruct("");
        });
        return arguments;
    });
    return listOfArguments;
}

void DataWarehouseManager::DataPrivate::insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values)
{
    std::scoped_lock<std::mutex> loc(mDatabaseMutex);
    if (auto it = mDatabaseWrapper.find(dbId); it != mDatabaseWrapper.end())
    {
        it->second->insertIntoDatabase(tableName, columnFields, convertToDatabaseArguments(values));
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

void DataWarehouseManager::insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values)
{
    for (const auto& value: values)
    {
        if (columnFields.size() != value.size())
        {
            SERVICE_LOG_WARN("insert data but column fields size not matched, dbId" << dbId);
            return;
        }
    }

    mDataPrivate->insertIntoDatabase(dbId, tableName, columnFields, values);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataWarehouseManager Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}