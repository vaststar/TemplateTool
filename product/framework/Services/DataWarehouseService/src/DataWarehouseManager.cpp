#include <map>
#include <mutex>
#include <algorithm>
#include <functional>
#include <filesystem>

#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DataBaseDataValue.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseSchema.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseDataRecord.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/IDatabaseWrapper.h>

#include <ucf/Services/DataWarehouseService/DataBaseConfig.h>
#include <ucf/Services/DataWarehouseService/DataBaseDataValue.h>
#include <ucf/Services/DataWarehouseService/DatabaseDataRecord.h>
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
    void fetchFromDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListsOfWhereCondition& whereConditions, model::DatabaseDataRecordsCallback func, int limit, const std::source_location location);

    void createFolder(const std::string& dbFilePath) const;
    std::shared_ptr<ucf::utilities::database::IDatabaseWrapper> getDBWrapper(const std::string& dbId);

private:
    ucf::utilities::database::DatabaseSchemas convertToDatabaseSchemas(const std::vector<model::DBTableModel>& tableModels) const;
    ucf::utilities::database::ListOfArguments convertToDatabaseArguments(const model::ListOfDBValues& values) const;
    ucf::utilities::database::DataBaseDataValue convertToDatabaseDataValue(const model::DataBaseDataValue& value) const;
    ucf::utilities::database::ListsOfWhereCondition convertToDatabaseWhereConditions(const model::ListsOfWhereCondition& conditions) const;
    ucf::utilities::database::DBOperatorType convertToDatabaseOperatorType(model::DBOperatorType operatorType) const;

    model::DataBaseDataValue convertFromDatabaseDataValue(const ucf::utilities::database::DataBaseDataValue& value) const;
    model::DatabaseDataRecords convertFromDatabaseRecords(const ucf::utilities::database::DatabaseDataRecords& records) const;
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

std::shared_ptr<ucf::utilities::database::IDatabaseWrapper> DataWarehouseManager::DataPrivate::getDBWrapper(const std::string& dbId)
{
    std::scoped_lock<std::mutex> loc(mDatabaseMutex);
    if (auto it = mDatabaseWrapper.find(dbId); it != mDatabaseWrapper.end())
    {
        return it->second;
    }
    return nullptr;
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

ucf::utilities::database::DataBaseDataValue DataWarehouseManager::DataPrivate::convertToDatabaseDataValue(const model::DataBaseDataValue& value) const
{
    if (value.holdsType<model::DBSupportedTypes::STRING>())
    {
        return ucf::utilities::database::DataBaseDataValue(value.getStringValue());
    }
    else if (value.holdsType<model::DBSupportedTypes::INT>())
    {
        return ucf::utilities::database::DataBaseDataValue(value.getIntValue());
    }
    else if (value.holdsType<model::DBSupportedTypes::FLOAT>())
    {
        return ucf::utilities::database::DataBaseDataValue(value.getFloatValue());
    }
    else if (value.holdsType<model::DBSupportedTypes::BUFFER>())
    {
        return ucf::utilities::database::DataBaseDataValue(value.getBufferValue());
    }
    else
    {
        SERVICE_LOG_WARN("convert data failed");
        return ucf::utilities::database::DataBaseDataValue("");
    }
}

ucf::utilities::database::ListOfArguments DataWarehouseManager::DataPrivate::convertToDatabaseArguments(const model::ListOfDBValues& values) const
{
    ucf::utilities::database::ListOfArguments listOfArguments;
    std::transform(values.cbegin(), values.cend(), std::back_inserter(listOfArguments), [this](const auto& value) {
        ucf::utilities::database::Arguments arguments; 
        std::transform(value.begin(), value.end(), std::back_inserter(arguments), std::bind_front(&DataWarehouseManager::DataPrivate::convertToDatabaseDataValue, this));
        return arguments;
    });
    return listOfArguments;
}

void DataWarehouseManager::DataPrivate::insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values, const std::source_location location)
{
    if (auto dbWrapper = getDBWrapper(dbId))
    {
        dbWrapper->insertIntoDatabase(tableName, columnFields, convertToDatabaseArguments(values), location);
    }
    else
    {
        SERVICE_LOG_WARN("no db:" << dbId);
    }
}

ucf::utilities::database::DBOperatorType DataWarehouseManager::DataPrivate::convertToDatabaseOperatorType(model::DBOperatorType operatorType) const
{
    switch (operatorType)
    {
    case model::DBOperatorType::Equal:
        return ucf::utilities::database::DBOperatorType::Equal;
    case model::DBOperatorType::Less:
        return ucf::utilities::database::DBOperatorType::Less;
    case model::DBOperatorType::Greater:
        return ucf::utilities::database::DBOperatorType::Greater;
    case model::DBOperatorType::Match:
        return ucf::utilities::database::DBOperatorType::Match;
    case model::DBOperatorType::In:
        return ucf::utilities::database::DBOperatorType::In;
    case model::DBOperatorType::NotIn:
        return ucf::utilities::database::DBOperatorType::NotIn;
    case model::DBOperatorType::And:
        return ucf::utilities::database::DBOperatorType::And;
    case model::DBOperatorType::Like:
        return ucf::utilities::database::DBOperatorType::Like;
    case model::DBOperatorType::Not:
        return ucf::utilities::database::DBOperatorType::Not;
    case model::DBOperatorType::IsNull:
        return ucf::utilities::database::DBOperatorType::IsNull;
    default:
        return ucf::utilities::database::DBOperatorType::Equal;
    }
}

ucf::utilities::database::ListsOfWhereCondition DataWarehouseManager::DataPrivate::convertToDatabaseWhereConditions(const model::ListsOfWhereCondition& conditions) const
{
    ucf::utilities::database::ListsOfWhereCondition dbWhereCondition;

    std::transform(conditions.cbegin(), conditions.cend(), std::back_inserter(dbWhereCondition), [this](const auto& condition){
        return std::make_tuple(std::get<0>(condition), convertToDatabaseDataValue(std::get<1>(condition)), convertToDatabaseOperatorType(std::get<2>(condition)));
    });
    return dbWhereCondition;
}

model::DataBaseDataValue DataWarehouseManager::DataPrivate::convertFromDatabaseDataValue(const ucf::utilities::database::DataBaseDataValue& value) const
{
    if (value.holdsType<ucf::utilities::database::DBSupportedTypes::STRING>())
    {
        return model::DataBaseDataValue(value.getStringValue());
    }
    else if (value.holdsType<ucf::utilities::database::DBSupportedTypes::INT>())
    {
        return model::DataBaseDataValue(value.getIntValue());
    }
    else if (value.holdsType<ucf::utilities::database::DBSupportedTypes::FLOAT>())
    {
        return model::DataBaseDataValue(value.getFloatValue());
    }
    else if (value.holdsType<ucf::utilities::database::DBSupportedTypes::BLOB>())
    {
        return model::DataBaseDataValue(value.getBufferValue());
    }
    else
    {
        SERVICE_LOG_WARN("convert data failed");
        return model::DataBaseDataValue("");
    }
}

model::DatabaseDataRecords DataWarehouseManager::DataPrivate::convertFromDatabaseRecords(const ucf::utilities::database::DatabaseDataRecords& records) const
{
    model::DatabaseDataRecords resultRecords;
    std::transform(records.cbegin(), records.cend(), std::back_inserter(resultRecords), [this](const auto& dbRecord){
        model::DatabaseDataRecord modelRecord;
        for (const auto&[columnName, columnData]: dbRecord.getData() )
        {
            modelRecord.addColumnData(columnName, convertFromDatabaseDataValue(columnData));
        }
        
        return modelRecord;
    });
    return resultRecords;
}

void DataWarehouseManager::DataPrivate::fetchFromDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListsOfWhereCondition& whereConditions, model::DatabaseDataRecordsCallback func, int limit, const std::source_location location)
{
    if (auto dbWrapper = getDBWrapper(dbId))
    {
        ucf::utilities::database::ListsOfWhereCondition dbQueryConditions = convertToDatabaseWhereConditions(whereConditions);
        ucf::utilities::database::DatabaseDataRecordsCallback dbWrapperCallback= [func, this](const ucf::utilities::database::DatabaseDataRecords& dbRecords){
            func(convertFromDatabaseRecords(dbRecords));
        };
        dbWrapper->fetchFromDatabase(tableName, columnFields, dbQueryConditions, dbWrapperCallback, limit, location);
    }
    else
    {
        SERVICE_LOG_WARN("no db:" << dbId);
        func({});
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

void DataWarehouseManager::fetchFromDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListsOfWhereCondition& whereConditions, model::DatabaseDataRecordsCallback func, int limit, const std::source_location location)
{
    if (dbId.empty() || tableName.empty())
    {
        SERVICE_LOG_WARN("wrong fetch param, dbId" << dbId << ", tableName: " << tableName);
        func({});
    }
    mDataPrivate->fetchFromDatabase(dbId, tableName, columnFields, whereConditions, func, limit, location);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataWarehouseManager Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}