
#include "DataWarehouseManager.h"

#include <map>
#include <mutex>
#include <algorithm>
#include <functional>
#include <filesystem>

#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseDataValue.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseSchema.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseDataRecord.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/IDatabaseWrapper.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseFactory.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseConfig.h>

#include <ucf/Services/DataWarehouseService/DatabaseConfig.h>
#include <ucf/Services/DataWarehouseService/DatabaseDataValue.h>
#include <ucf/Services/DataWarehouseService/DatabaseDataRecord.h>
#include <ucf/Services/DataWarehouseService/DatabaseTableModel.h>

#include "DataWarehouseServiceLogger.h"

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
    bool insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values, const std::source_location location);
    void fetchFromDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListsOfWhereCondition& whereConditions, model::DatabaseDataRecordsCallback func, int limit, const std::source_location location);
    int64_t updateInDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::DBDataValues& values, const model::ListsOfWhereCondition& whereConditions, const std::source_location location);
    bool updateBatch(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& keyColumns, const model::DBColumnFields& valueColumns, const model::ListOfDBValues& items, const std::source_location location);
    int64_t deleteFromDatabase(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions, const std::source_location location);
    bool exists(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions);
    int64_t count(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions);

    void createFolder(const std::string& dbFilePath) const;
    std::shared_ptr<ucf::utilities::database::IDatabaseWrapper> getDBWrapper(const std::string& dbId);

private:
    ucf::utilities::database::DatabaseSchemas convertToDatabaseSchemas(const std::vector<model::DBTableModel>& tableModels) const;
    ucf::utilities::database::ListOfArguments convertToDatabaseArguments(const model::ListOfDBValues& values) const;
    ucf::utilities::database::DatabaseDataValue convertToDatabaseDataValue(const model::DatabaseDataValue& value) const;
    ucf::utilities::database::ListsOfWhereCondition convertToDatabaseWhereConditions(const model::ListsOfWhereCondition& conditions) const;
    ucf::utilities::database::DBOperatorType convertToDatabaseOperatorType(model::DBOperatorType operatorType) const;

    model::DatabaseDataValue convertFromDatabaseDataValue(const ucf::utilities::database::DatabaseDataValue& value) const;
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
            ucf::utilities::database::SqliteDatabaseConfig config;
            config.fileName = sqliteConfig->getDBFilePath();
            config.password = sqliteConfig->getDBPassword();
            auto dataBaseWrapper = ucf::utilities::database::DatabaseFactory::create(config);
            
            if (!dataBaseWrapper->open())
            {
                SERVICE_LOG_ERROR("failed to open database: " << dbConfig->getDBId());
                return;
            }
            
            mDatabaseWrapper[dbConfig->getDBId()] = dataBaseWrapper;
            if (!tables.empty())
            {
                dataBaseWrapper->createTables(convertToDatabaseSchemas(tables));
            }
            SERVICE_LOG_INFO("database created and opened: " << dbConfig->getDBId());
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

ucf::utilities::database::DatabaseDataValue DataWarehouseManager::DataPrivate::convertToDatabaseDataValue(const model::DatabaseDataValue& value) const
{
    if (value.holdsType<model::DBSupportedTypes::STRING>())
    {
        return ucf::utilities::database::DatabaseDataValue(value.getStringValue());
    }
    else if (value.holdsType<model::DBSupportedTypes::INT>())
    {
        return ucf::utilities::database::DatabaseDataValue(value.getIntValue());
    }
    else if (value.holdsType<model::DBSupportedTypes::FLOAT>())
    {
        return ucf::utilities::database::DatabaseDataValue(value.getFloatValue());
    }
    else if (value.holdsType<model::DBSupportedTypes::BUFFER>())
    {
        return ucf::utilities::database::DatabaseDataValue(value.getBufferValue());
    }
    else
    {
        SERVICE_LOG_WARN("convert data failed");
        return ucf::utilities::database::DatabaseDataValue("");
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

bool DataWarehouseManager::DataPrivate::insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values, const std::source_location location)
{
    if (auto dbWrapper = getDBWrapper(dbId))
    {
        auto arguments = convertToDatabaseArguments(values);
        if (arguments.empty())
        {
            return true;  // Empty data is considered success
        }
        else if (arguments.size() == 1)
        {
            // Single row - no transaction needed
            return dbWrapper->insertIntoDatabase(tableName, columnFields, arguments[0], location);
        }
        else
        {
            // Multiple rows - use batch insert with transaction
            return dbWrapper->insertBatch(tableName, columnFields, arguments, location);
        }
    }
    SERVICE_LOG_WARN("no db:" << dbId);
    return false;
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

model::DatabaseDataValue DataWarehouseManager::DataPrivate::convertFromDatabaseDataValue(const ucf::utilities::database::DatabaseDataValue& value) const
{
    if (value.holdsType<ucf::utilities::database::DBSupportedTypes::STRING>())
    {
        return model::DatabaseDataValue(value.getStringValue());
    }
    else if (value.holdsType<ucf::utilities::database::DBSupportedTypes::INT>())
    {
        return model::DatabaseDataValue(value.getIntValue());
    }
    else if (value.holdsType<ucf::utilities::database::DBSupportedTypes::FLOAT>())
    {
        return model::DatabaseDataValue(value.getFloatValue());
    }
    else if (value.holdsType<ucf::utilities::database::DBSupportedTypes::BLOB>())
    {
        return model::DatabaseDataValue(value.getBufferValue());
    }
    else
    {
        SERVICE_LOG_WARN("convert data failed");
        return model::DatabaseDataValue("");
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

int64_t DataWarehouseManager::DataPrivate::updateInDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::DBDataValues& values, const model::ListsOfWhereCondition& whereConditions, const std::source_location location)
{
    if (auto dbWrapper = getDBWrapper(dbId))
    {
        ucf::utilities::database::Arguments dbValues;
        std::transform(values.cbegin(), values.cend(), std::back_inserter(dbValues), 
            std::bind_front(&DataWarehouseManager::DataPrivate::convertToDatabaseDataValue, this));
        ucf::utilities::database::ListsOfWhereCondition dbConditions = convertToDatabaseWhereConditions(whereConditions);
        return dbWrapper->updateInDatabase(tableName, columnFields, dbValues, dbConditions, location);
    }
    SERVICE_LOG_WARN("no db:" << dbId);
    return -1;
}

bool DataWarehouseManager::DataPrivate::updateBatch(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& keyColumns, const model::DBColumnFields& valueColumns, const model::ListOfDBValues& items, const std::source_location location)
{
    if (auto dbWrapper = getDBWrapper(dbId))
    {
        auto dbItems = convertToDatabaseArguments(items);
        return dbWrapper->updateBatch(tableName, keyColumns, valueColumns, dbItems, location);
    }
    SERVICE_LOG_WARN("no db:" << dbId);
    return false;
}

int64_t DataWarehouseManager::DataPrivate::deleteFromDatabase(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions, const std::source_location location)
{
    if (auto dbWrapper = getDBWrapper(dbId))
    {
        ucf::utilities::database::ListsOfWhereCondition dbConditions = convertToDatabaseWhereConditions(whereConditions);
        return dbWrapper->deleteFromDatabase(tableName, dbConditions, location);
    }
    SERVICE_LOG_WARN("no db:" << dbId);
    return -1;
}

bool DataWarehouseManager::DataPrivate::exists(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions)
{
    if (auto dbWrapper = getDBWrapper(dbId))
    {
        ucf::utilities::database::ListsOfWhereCondition dbConditions = convertToDatabaseWhereConditions(whereConditions);
        return dbWrapper->exists(tableName, dbConditions);
    }
    SERVICE_LOG_WARN("no db:" << dbId);
    return false;
}

int64_t DataWarehouseManager::DataPrivate::count(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions)
{
    if (auto dbWrapper = getDBWrapper(dbId))
    {
        ucf::utilities::database::ListsOfWhereCondition dbConditions = convertToDatabaseWhereConditions(whereConditions);
        return dbWrapper->count(tableName, dbConditions);
    }
    SERVICE_LOG_WARN("no db:" << dbId);
    return -1;
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

bool DataWarehouseManager::insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values, const std::source_location location)
{
    for (const auto& value: values)
    {
        if (columnFields.size() != value.size())
        {
            SERVICE_LOG_WARN("insert data but column fields size not matched, dbId" << dbId);
            return false;
        }
    }

    return mDataPrivate->insertIntoDatabase(dbId, tableName, columnFields, values, location);
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

int64_t DataWarehouseManager::updateInDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::DBDataValues& values, const model::ListsOfWhereCondition& whereConditions, const std::source_location location)
{
    if (dbId.empty() || tableName.empty())
    {
        SERVICE_LOG_WARN("wrong update param, dbId" << dbId << ", tableName: " << tableName);
        return -1;
    }
    if (columnFields.size() != values.size())
    {
        SERVICE_LOG_WARN("update data but column fields size not matched, dbId" << dbId);
        return -1;
    }
    return mDataPrivate->updateInDatabase(dbId, tableName, columnFields, values, whereConditions, location);
}

bool DataWarehouseManager::updateBatch(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& keyColumns, const model::DBColumnFields& valueColumns, const model::ListOfDBValues& items, const std::source_location location)
{
    if (dbId.empty() || tableName.empty())
    {
        SERVICE_LOG_WARN("wrong updateBatch param, dbId" << dbId << ", tableName: " << tableName);
        return false;
    }
    if (keyColumns.empty() || valueColumns.empty())
    {
        SERVICE_LOG_WARN("updateBatch with empty columns, dbId" << dbId);
        return false;
    }
    size_t expectedSize = keyColumns.size() + valueColumns.size();
    for (const auto& item : items)
    {
        if (item.size() != expectedSize)
        {
            SERVICE_LOG_WARN("updateBatch item size mismatch, expected: " << expectedSize << ", got: " << item.size() << ", dbId: " << dbId);
            return false;
        }
    }
    return mDataPrivate->updateBatch(dbId, tableName, keyColumns, valueColumns, items, location);
}

int64_t DataWarehouseManager::deleteFromDatabase(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions, const std::source_location location)
{
    if (dbId.empty() || tableName.empty())
    {
        SERVICE_LOG_WARN("wrong delete param, dbId" << dbId << ", tableName: " << tableName);
        return -1;
    }
    return mDataPrivate->deleteFromDatabase(dbId, tableName, whereConditions, location);
}

bool DataWarehouseManager::exists(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions)
{
    if (dbId.empty() || tableName.empty())
    {
        SERVICE_LOG_WARN("wrong exists param, dbId" << dbId << ", tableName: " << tableName);
        return false;
    }
    return mDataPrivate->exists(dbId, tableName, whereConditions);
}

int64_t DataWarehouseManager::count(const std::string& dbId, const std::string& tableName, const model::ListsOfWhereCondition& whereConditions)
{
    if (dbId.empty() || tableName.empty())
    {
        SERVICE_LOG_WARN("wrong count param, dbId" << dbId << ", tableName: " << tableName);
        return -1;
    }
    return mDataPrivate->count(dbId, tableName, whereConditions);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataWarehouseManager Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}