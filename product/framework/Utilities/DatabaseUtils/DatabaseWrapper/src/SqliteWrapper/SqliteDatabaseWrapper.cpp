#include <sstream>
#include <algorithm>
#include <numeric>
#include <mutex>

#include <sqlite3.h>

#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseDataValue.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseSchema.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseDataRecord.h>

#include "SqliteWrapper/SqliteDatabaseWrapper.h"
#include "DatabaseWrapperLogger.h"

namespace ucf::utilities::database{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

class SqliteDatabaseWrapper::DataPrivate
{
public:
    explicit DataPrivate(const SqliteDatabaseConfig& config);
    bool openDatabase();
    void closeDatabase();
    bool isOpen();
    bool execute(const std::string& commandStr);
    bool prepareStatement(const std::string& statement, sqlite3_stmt** ppStmt);
    bool bindDBType(sqlite3_stmt* statement, const DatabaseDataValue& value, int index);
    void extractResultsFromStatement(sqlite3_stmt* statement, DatabaseDataRecords& result);
private:
    mutable std::mutex mDatabaseMutex;  // Thread safety for database access
    SqliteDatabaseConfig mDatabaseConfig;
    sqlite3* mDatabase;
};

SqliteDatabaseWrapper::DataPrivate::DataPrivate(const SqliteDatabaseConfig& config)
    : mDatabaseConfig(config)
    , mDatabase(nullptr)
{
}

bool SqliteDatabaseWrapper::DataPrivate::openDatabase()
{
    std::lock_guard<std::mutex> lock(mDatabaseMutex);
    
    if (mDatabase) 
    {
        DBWRAPPER_LOG_WARN("database already open, dbname: " << mDatabaseConfig.fileName);
        return true;
    }
    
    // Use FULLMUTEX for thread safety within SQLite itself
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
    int result = sqlite3_open_v2(mDatabaseConfig.fileName.c_str(), &mDatabase, flags, nullptr);
    
    if (SQLITE_OK != result)
    {
        std::string errorMsg = mDatabase ? sqlite3_errmsg(mDatabase) : "unknown error";
        DBWRAPPER_LOG_ERROR("open db failed, dbname: " << mDatabaseConfig.fileName 
                          << ", error code: " << result 
                          << ", error: " << errorMsg);
        if (mDatabase) 
        {
            sqlite3_close(mDatabase);
            mDatabase = nullptr;
        }
        return false;
    }
    
    DBWRAPPER_LOG_INFO("open db successfully, dbname: " << mDatabaseConfig.fileName);
    
    // Enable WAL mode for better concurrency
    char* sqlError = nullptr;
    if (SQLITE_OK != sqlite3_exec(mDatabase, "PRAGMA journal_mode=WAL", nullptr, nullptr, &sqlError))
    {
        DBWRAPPER_LOG_WARN("failed to enable WAL mode: " << (sqlError ? sqlError : "unknown"));
        if (sqlError) sqlite3_free(sqlError);
    }
    else
    {
        DBWRAPPER_LOG_DEBUG("WAL mode enabled for: " << mDatabaseConfig.fileName);
    }
    
    // Set synchronous mode for performance/safety balance
    if (SQLITE_OK != sqlite3_exec(mDatabase, "PRAGMA synchronous=NORMAL", nullptr, nullptr, &sqlError))
    {
        DBWRAPPER_LOG_WARN("failed to set synchronous mode: " << (sqlError ? sqlError : "unknown"));
        if (sqlError) sqlite3_free(sqlError);
    }
    
    return true;
}

void SqliteDatabaseWrapper::DataPrivate::closeDatabase()
{
    std::lock_guard<std::mutex> lock(mDatabaseMutex);
    
    if (mDatabase)
    {
        int result = sqlite3_close(mDatabase);
        if (SQLITE_OK != result)
        {
            DBWRAPPER_LOG_WARN("close db warning, dbname: " << mDatabaseConfig.fileName 
                             << ", error code: " << result);
        }
        else
        {
            DBWRAPPER_LOG_INFO("close db successfully, dbname: " << mDatabaseConfig.fileName);
        }
        mDatabase = nullptr;
    }
}

bool SqliteDatabaseWrapper::DataPrivate::isOpen()
{
    std::lock_guard<std::mutex> lock(mDatabaseMutex);
    return nullptr != mDatabase;
}

bool SqliteDatabaseWrapper::DataPrivate::execute(const std::string& commandStr)
{
    std::lock_guard<std::mutex> lock(mDatabaseMutex);
    
    if (!mDatabase)
    {
        DBWRAPPER_LOG_ERROR("execute failed: database not open");
        return false;
    }
    
    char* sqlError = nullptr;
    int result = sqlite3_exec(mDatabase, commandStr.c_str(), nullptr, nullptr, &sqlError);
    if (SQLITE_OK != result)
    {
        std::string errorMessage = sqlError ? sqlError : "unknown error";
        if (sqlError) sqlite3_free(sqlError);
        DBWRAPPER_LOG_ERROR("execute db command failed, dbname: " << mDatabaseConfig.fileName 
                          << ", result code: " << result 
                          << ", error: " << errorMessage
                          << ", sql: " << commandStr.substr(0, 100));
        return false;
    }
    return true;
}

bool SqliteDatabaseWrapper::DataPrivate::prepareStatement(const std::string& statement, sqlite3_stmt** ppStmt)
{
    std::lock_guard<std::mutex> lock(mDatabaseMutex);
    
    if (!mDatabase)
    {
        DBWRAPPER_LOG_ERROR("prepareStatement failed: database not open");
        return false;
    }
    
    int result = sqlite3_prepare_v2(mDatabase, statement.c_str(), static_cast<int>(statement.size()), ppStmt, nullptr);
    if (SQLITE_OK != result)
    {
        DBWRAPPER_LOG_ERROR("prepare statement failed, error code: " << result 
                          << ", error: " << sqlite3_errmsg(mDatabase)
                          << ", sql: " << statement.substr(0, 100));
        return false;
    }
    return true;
}

bool SqliteDatabaseWrapper::DataPrivate::bindDBType(sqlite3_stmt* statement, const DatabaseDataValue& value, int index)
{
    // Handle NULL type first
    if (value.isNull())
    {
        if (auto result = sqlite3_bind_null(statement, index); SQLITE_OK == result)
        {
            return true;
        }
        else
        {
            DBWRAPPER_LOG_ERROR("db bind null failed, error code: " << result);
            return false;
        }
    }
    
    if (value.holdsType<DBSupportedTypes::STRING>())
    {
        const auto str = value.getStringValue();
        if (auto result = sqlite3_bind_text(statement, index, str.c_str(), static_cast<int>(str.size()), SQLITE_TRANSIENT);  SQLITE_OK == result)
        {
            return true;
        }
        else
        {
            DBWRAPPER_LOG_WARN("db bind text failed, error code: " << result);
            return false;
        }
    }
    else if(value.holdsType<DBSupportedTypes::INT>())
    {
        if (auto result = sqlite3_bind_int64(statement, index, value.getIntValue()); SQLITE_OK == result)
        {
            return true;
        }
        else
        {
            DBWRAPPER_LOG_WARN("db bind int failed, error code: " << result);
            return false;
        }
    }
    else if(value.holdsType<DBSupportedTypes::FLOAT>())
    {
        if (auto result = sqlite3_bind_double(statement, index, static_cast<double>(value.getFloatValue()));  SQLITE_OK == result)
        {
            return true;
        }
        else
        {
            DBWRAPPER_LOG_WARN("db bind float failed, error code: " << result);
            return false;
        }
    }
    else if(value.holdsType<DBSupportedTypes::BLOB>())
    {
        const auto bufferData = value.getBufferValue();
        if (bufferData.empty())
        {
            if (auto result = sqlite3_bind_blob(statement, index, nullptr, 0, SQLITE_STATIC); SQLITE_OK == result)
            {
                return true;
            }
            else
            {
                DBWRAPPER_LOG_WARN("db empty blob failed, error code: " << result);
                return false;
            }
        }
        else
        {
            // Use SQLITE_TRANSIENT to let SQLite copy the data, avoiding dangling pointer
            if (auto result = sqlite3_bind_blob(statement, index, bufferData.data(), static_cast<int>(bufferData.size()), SQLITE_TRANSIENT); SQLITE_OK == result)
            {
                return true;
            }
            else
            {
                DBWRAPPER_LOG_WARN("db blob failed, error code: " << result);
                return false;
            }
        }
    }
    DBWRAPPER_LOG_WARN("db bind text failed");
    return false;
}

void SqliteDatabaseWrapper::DataPrivate::extractResultsFromStatement(sqlite3_stmt* statement, DatabaseDataRecords& result)
{
    while (SQLITE_ROW == sqlite3_step(statement))
    {
        int numberOfColumns = sqlite3_data_count(statement);
        DatabaseDataRecord oneRowResult;
        for (int i = 0; i < numberOfColumns; ++i)
        {
            std::string columnName = sqlite3_column_name(statement, i);
            int columnType = sqlite3_column_type(statement, i);
            switch (columnType)
            {
            case SQLITE_TEXT:
            {
                const unsigned char* columnText = sqlite3_column_text(statement, i);
                const char* columnTextChar = (columnText == nullptr) ? "NULL" : reinterpret_cast<const char*>(columnText);
                oneRowResult.addColumnData(columnName, columnTextChar);
            }
                break;
            case SQLITE_INTEGER:
            {
                oneRowResult.addColumnData(columnName, sqlite3_column_int64(statement, i));
            }
                break;
            case SQLITE_BLOB:
            {
                unsigned char* blob = (unsigned char*)sqlite3_column_blob(statement, i);
                int size = sqlite3_column_bytes(statement, i);
                if (blob != nullptr && size > 0)
                {
                    oneRowResult.addColumnData(columnName, DBSupportedTypes::BLOB(blob, blob + size));
                }
                else
                {
                    DBWRAPPER_LOG_WARN("Blob seems to be invalid when reading back from the DB.");
                    oneRowResult.addColumnData(columnName, DBSupportedTypes::BLOB());
                }
            }
                break;
            case SQLITE_FLOAT:
            {
                oneRowResult.addColumnData(columnName, static_cast<float>(sqlite3_column_double(statement, i)));
            }
                break;
            case SQLITE_NULL:
            {
                oneRowResult.addColumnData(columnName, DatabaseDataValue{});
            }
                break;
            default:
            {
                DBWRAPPER_LOG_WARN("Invalid column type found: " << columnType);
                oneRowResult.addColumnData(columnName, ""); // fall back to empty string
            }
                break;
            }
        }
        result.emplace_back(std::move(oneRowResult));
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start SqliteDatabaseWrapper Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
SqliteDatabaseWrapper::SqliteDatabaseWrapper(const SqliteDatabaseConfig& config)
    : mDataPrivate(std::make_unique<SqliteDatabaseWrapper::DataPrivate>(config))
{
    DBWRAPPER_LOG_INFO("create SqliteDatabaseWrapper");
}

SqliteDatabaseWrapper::~SqliteDatabaseWrapper()
{
    DBWRAPPER_LOG_INFO("delete SqliteDatabaseWrapper");
    try
    {
        close();
    }
    catch(const std::exception& e)
    {
        DBWRAPPER_LOG_ERROR("close db exception: " << e.what()); 
    }
    catch(...)
    {
        DBWRAPPER_LOG_ERROR("close db unknown exception"); 
    }
}

bool SqliteDatabaseWrapper::open()
{
    return mDataPrivate->openDatabase();
}

void SqliteDatabaseWrapper::close()
{
    mDataPrivate->closeDatabase();
}

bool SqliteDatabaseWrapper::isOpen()
{
    return mDataPrivate->isOpen();
}

bool SqliteDatabaseWrapper::beginTransaction()
{
    DBWRAPPER_LOG_DEBUG("begin transaction");
    return mDataPrivate->execute("BEGIN IMMEDIATE TRANSACTION");
}

bool SqliteDatabaseWrapper::commit()
{
    DBWRAPPER_LOG_DEBUG("commit transaction");
    return mDataPrivate->execute("COMMIT");
}

bool SqliteDatabaseWrapper::rollback()
{
    DBWRAPPER_LOG_WARN("rollback transaction");
    return mDataPrivate->execute("ROLLBACK");
}

void SqliteDatabaseWrapper::createTables(const DatabaseSchemas& tableSchemas)
{
    for (const auto& tableInfo : tableSchemas)
    {
        std::string createStatement = "CREATE TABLE IF NOT EXISTS " + tableInfo.tableName() + tableInfo.schema();
        mDataPrivate->execute(createStatement);
    }
}

bool SqliteDatabaseWrapper::insertIntoDatabase(const std::string& tableName, const Columns& columns, const Arguments& arguments, const std::source_location location)
{
    DBWRAPPER_LOG_DEBUG("insert single row into table: " << tableName << ", from: " 
              << location.file_name() << '('
              << location.line() << ':'
              << location.column() << ") `"
              << location.function_name());

    if (columns.empty() || arguments.empty())
    {
        DBWRAPPER_LOG_WARN("insert empty data into table: " << tableName);
        return false;
    }

    if (columns.size() != arguments.size())
    {
        DBWRAPPER_LOG_WARN("mismatch in columns and data, table: " << tableName);
        return false;
    }
    
    std::string insertStatement = generateInsertStatement(tableName, columns);

    sqlite3_stmt* statement = nullptr;
    if (!mDataPrivate->prepareStatement(insertStatement, &statement))
    {
        DBWRAPPER_LOG_WARN("prepare statement failed, table: " << tableName);
        return false;
    }

    bool success = true;
    for (size_t i = 0; i < arguments.size(); ++i)
    {
        if (!mDataPrivate->bindDBType(statement, arguments[i], static_cast<int>(i + 1)))
        {
            DBWRAPPER_LOG_WARN("bind statement failed, table: " << tableName);
            success = false;
            break;
        }
    }

    if (success)
    {
        if (auto result = sqlite3_step(statement); SQLITE_OK != result && SQLITE_DONE != result)
        {
            DBWRAPPER_LOG_WARN("step statement failed, table: " << tableName << ", error code: " << result);
            success = false;
        }
    }

    if (auto result = sqlite3_finalize(statement); SQLITE_OK != result && SQLITE_DONE != result)
    {
        DBWRAPPER_LOG_WARN("finalize statement failed, table: " << tableName << ", error code: " << result);
        return false;
    }
    
    return success;
}

bool SqliteDatabaseWrapper::insertBatch(const std::string& tableName, const Columns& columns, const ListOfArguments& arguments, const std::source_location location)
{
    DBWRAPPER_LOG_DEBUG("batch insert into table: " << tableName 
              << ", rows: " << arguments.size()
              << ", from: " << location.file_name() << '(' << location.line() << ')');

    if (columns.empty() || arguments.empty())
    {
        DBWRAPPER_LOG_WARN("insert empty data into table: " << tableName);
        return false;
    }

    for (const auto& argument : arguments)
    {
        if (columns.size() != argument.size())
        {
            DBWRAPPER_LOG_ERROR("mismatch in columns and data, table: " << tableName);
            return false;
        }
    }

    // Use transaction for atomicity and performance
    if (!beginTransaction())
    {
        DBWRAPPER_LOG_ERROR("failed to begin transaction for batch insert, table: " << tableName);
        return false;
    }

    std::string insertStatement = generateInsertStatement(tableName, columns);
    sqlite3_stmt* statement = nullptr;
    if (!mDataPrivate->prepareStatement(insertStatement, &statement))
    {
        DBWRAPPER_LOG_ERROR("prepare statement failed for batch insert, table: " << tableName);
        rollback();
        return false;
    }

    bool success = true;
    size_t insertedCount = 0;
    
    for (const auto& argument : arguments)
    {
        for (size_t i = 0; i < argument.size(); ++i)
        {
            if (!mDataPrivate->bindDBType(statement, argument[i], static_cast<int>(i + 1)))
            {
                DBWRAPPER_LOG_ERROR("bind failed at row " << insertedCount << ", table: " << tableName);
                success = false;
                break;
            }
        }
        
        if (!success) break;

        if (auto result = sqlite3_step(statement); SQLITE_OK != result && SQLITE_DONE != result)
        {
            DBWRAPPER_LOG_ERROR("step failed at row " << insertedCount << ", table: " << tableName << ", error: " << result);
            success = false;
            break;
        }

        if (auto result = sqlite3_reset(statement); SQLITE_OK != result)
        {
            DBWRAPPER_LOG_ERROR("reset failed at row " << insertedCount << ", table: " << tableName << ", error: " << result);
            success = false;
            break;
        }
        
        ++insertedCount;
    }

    sqlite3_finalize(statement);

    if (success)
    {
        if (commit())
        {
            DBWRAPPER_LOG_INFO("batch insert completed, table: " << tableName << ", rows: " << insertedCount);
            return true;
        }
        else
        {
            DBWRAPPER_LOG_ERROR("commit failed for batch insert, table: " << tableName);
            return false;
        }
    }
    else
    {
        DBWRAPPER_LOG_WARN("batch insert failed, rolling back, table: " << tableName << ", inserted before failure: " << insertedCount);
        rollback();
        return false;
    }
}

std::string SqliteDatabaseWrapper::generateInsertStatement(const std::string& tableName, const Columns& columns) const
{
    std::stringstream insertStatement;

    std::string fieldsStatement = std::string("(") + std::accumulate(std::next(columns.begin()), columns.end(), columns.front(), [](std::string a, std::string b){return a+","+b;}) + ")";
    
    std::vector<std::string> columnFileds;
    columnFileds.reserve(columns.size());
    int currentIndex = 1;
    std::transform(columns.cbegin(), columns.cend(), std::back_inserter(columnFileds), [&currentIndex](const std::string& ) {
        return "?" + std::to_string(currentIndex++);
    });

    std::string columnPlaceholderStatement= std::string("(") + std::accumulate(std::next(columnFileds.begin()), columnFileds.end(), columnFileds.front(), [](std::string a, std::string b){return a+","+b;}) + ")";

    insertStatement << "INSERT OR REPLACE INTO " << tableName << fieldsStatement << "VALUES" << columnPlaceholderStatement;
    return insertStatement.str();
}

std::string SqliteDatabaseWrapper::generateSelectStatement(const std::string& tableName, const Columns& columns, const ListsOfWhereCondition& arguments, size_t limit) const
{
    std::stringstream selectStatement;
    //select string
    if (columns.empty())
    {
        selectStatement << "SELECT * FROM " << tableName; 
    }
    else
    {
        std::string columnsString = std::accumulate(std::next(columns.begin()), columns.end(), columns.front(),[](std::string a, std::string b){ return a + "," + b;});
        selectStatement << "SELECT " << columnsString << " FROM " << tableName;
    }
    //whre condition
    selectStatement << " " << createWhereCondition(arguments);
    if (limit > 0)
    {
        selectStatement << " LIMIT " << limit;
    }
    return selectStatement.str();
}

std::string SqliteDatabaseWrapper::createWhereCondition(const ListsOfWhereCondition& arguments) const
{
    if (arguments.empty())
    {
        return {};
    }

    std::stringstream whereStatement;
    unsigned int index = 1;

    std::map<std::string, std::vector<std::pair<WhereCondition, int>>> groupedWhereConditions;
    for (size_t i = 0; i < arguments.size(); ++i)
    {
        std::string name = std::get<0>(arguments[i]);
        if (auto it = groupedWhereConditions.find(name); it != groupedWhereConditions.end())
        {
            it->second.emplace_back( arguments[i], static_cast<int>(i + 1));
        }
        else
        {
            groupedWhereConditions[name] = { { arguments[i], static_cast<int>(i + 1) } };
        }
    }
    
    whereStatement << " WHERE ";
    // for (size_t groupIndex = 0; groupIndex < groupedWhereConditions.size(); ++groupIndex)
    size_t indexInGroup = 1;
    for (const auto& [name, group] : groupedWhereConditions)
    {
        whereStatement << " ( ";

        size_t currentIndex = 1;
        for (const auto& [condition, conditionIndex] : group)
        {
            const auto& tuple = condition;

            whereStatement << std::get<0>(tuple);

            if (std::get<2>(tuple) == DBOperatorType::Equal)
            {
                whereStatement << " = ?" << conditionIndex;
            }
            else if (std::get<2>(tuple) == DBOperatorType::Less)
            {
                whereStatement << " < ?" << conditionIndex;
            }
            else if (std::get<2>(tuple) == DBOperatorType::Greater)
            {
                whereStatement << " > ?" << conditionIndex;
            }
            else if (std::get<2>(tuple) == DBOperatorType::Match)
            {
                whereStatement << " MATCH ?" << conditionIndex;
            }
            else if (std::get<2>(tuple) == DBOperatorType::And)
            {
                whereStatement << " & ?" << conditionIndex;
            }
            else if (std::get<2>(tuple) == DBOperatorType::Like)
            {
                whereStatement << " LIKE ?" << conditionIndex;
            }
            else if (std::get<2>(tuple) == DBOperatorType::Not)
            {
                whereStatement << " <> ?" << conditionIndex;
            }
            else if ((std::get<2>(tuple) == DBOperatorType::In) || (std::get<2>(tuple) == DBOperatorType::NotIn))
            {
                if (std::get<2>(tuple) == DBOperatorType::NotIn)
                {
                    whereStatement << " NOT ";
                }
                whereStatement << " IN(" << conditionIndex << ")";
            }
            else
            {
                DBWRAPPER_LOG_WARN("Invalid operator in fetchFromDatabase");
                return "";
            }

            if (currentIndex < group.size())
            {
                whereStatement << " OR ";
            }

            ++currentIndex;
        }

        whereStatement << " ) ";

        if (indexInGroup < groupedWhereConditions.size())
        {
            whereStatement << " AND ";
        }

        ++indexInGroup;
    }
    
    return whereStatement.str();
}

void SqliteDatabaseWrapper::fetchFromDatabase(const std::string& tableName, const Columns& columns, const ListsOfWhereCondition& arguments, DatabaseDataRecordsCallback func, size_t limit, const std::source_location location)
{
    DBWRAPPER_LOG_DEBUG("fetch data from table: " << tableName << ", from: " 
              << location.file_name() << '('
              << location.line() << ':'
              << location.column() << ") `"
              << location.function_name());
    std::string selectStatement = generateSelectStatement(tableName, columns, arguments, limit);
    sqlite3_stmt* statement = nullptr;
    if (!mDataPrivate->prepareStatement(selectStatement, &statement))
    {
        DBWRAPPER_LOG_WARN("prepare statment failed, table: " << tableName);
        func({});
        return;
    }


    for(size_t i = 0; i < arguments.size(); ++i)
    {
        if (!mDataPrivate->bindDBType(statement, std::get<1>(arguments[i]), i+1))
        {
            DBWRAPPER_LOG_WARN("bind statment failed, table: " << tableName << ", index: " << i);
            func({});
            return;
        }
    }

    DatabaseDataRecords result;
    mDataPrivate->extractResultsFromStatement(statement, result);

    if (auto result = sqlite3_finalize(statement); SQLITE_OK != result && SQLITE_DONE != result)
    {
        DBWRAPPER_LOG_WARN("finalize statment failed, table: " << tableName << ", error code: " << result);
        func({});
        return;
    }

    DBWRAPPER_LOG_DEBUG("fetch data succeed, table: " << tableName);
    func(result);
}

std::string SqliteDatabaseWrapper::generateUpdateStatement(const std::string& tableName, const Columns& columns, const ListsOfWhereCondition& conditions) const
{
    std::stringstream updateStatement;
    updateStatement << "UPDATE " << tableName << " SET ";
    
    for (size_t i = 0; i < columns.size(); ++i)
    {
        updateStatement << columns[i] << " = ?" << (i + 1);
        if (i + 1 < columns.size())
        {
            updateStatement << ", ";
        }
    }
    
    // WHERE conditions use indices after the SET columns
    if (!conditions.empty())
    {
        updateStatement << " WHERE ";
        for (size_t i = 0; i < conditions.size(); ++i)
        {
            const auto& [colName, value, op] = conditions[i];
            size_t paramIndex = columns.size() + i + 1;
            
            updateStatement << colName;
            switch (op)
            {
                case DBOperatorType::Equal:   updateStatement << " = ?" << paramIndex; break;
                case DBOperatorType::Less:    updateStatement << " < ?" << paramIndex; break;
                case DBOperatorType::Greater: updateStatement << " > ?" << paramIndex; break;
                case DBOperatorType::Not:     updateStatement << " <> ?" << paramIndex; break;
                case DBOperatorType::Like:    updateStatement << " LIKE ?" << paramIndex; break;
                case DBOperatorType::IsNull:  updateStatement << " IS NULL"; break;
                default:                      updateStatement << " = ?" << paramIndex; break;
            }
            
            if (i + 1 < conditions.size())
            {
                updateStatement << " AND ";
            }
        }
    }
    
    return updateStatement.str();
}

std::string SqliteDatabaseWrapper::generateDeleteStatement(const std::string& tableName, const ListsOfWhereCondition& conditions) const
{
    std::stringstream deleteStatement;
    deleteStatement << "DELETE FROM " << tableName;
    
    if (!conditions.empty())
    {
        deleteStatement << " WHERE ";
        for (size_t i = 0; i < conditions.size(); ++i)
        {
            const auto& [colName, value, op] = conditions[i];
            size_t paramIndex = i + 1;
            
            deleteStatement << colName;
            switch (op)
            {
                case DBOperatorType::Equal:   deleteStatement << " = ?" << paramIndex; break;
                case DBOperatorType::Less:    deleteStatement << " < ?" << paramIndex; break;
                case DBOperatorType::Greater: deleteStatement << " > ?" << paramIndex; break;
                case DBOperatorType::Not:     deleteStatement << " <> ?" << paramIndex; break;
                case DBOperatorType::Like:    deleteStatement << " LIKE ?" << paramIndex; break;
                case DBOperatorType::IsNull:  deleteStatement << " IS NULL"; break;
                default:                      deleteStatement << " = ?" << paramIndex; break;
            }
            
            if (i + 1 < conditions.size())
            {
                deleteStatement << " AND ";
            }
        }
    }
    
    return deleteStatement.str();
}

int64_t SqliteDatabaseWrapper::updateInDatabase(const std::string& tableName, const Columns& columns, const Arguments& values, const ListsOfWhereCondition& conditions, const std::source_location location)
{
    DBWRAPPER_LOG_DEBUG("update table: " << tableName << ", from: " 
              << location.file_name() << '('
              << location.line() << ':'
              << location.column() << ") `"
              << location.function_name());

    if (columns.empty() || values.empty())
    {
        DBWRAPPER_LOG_WARN("update with empty data, table: " << tableName);
        return -1;
    }

    if (columns.size() != values.size())
    {
        DBWRAPPER_LOG_WARN("mismatch in columns and values, table: " << tableName);
        return -1;
    }
    
    std::string updateStatement = generateUpdateStatement(tableName, columns, conditions);

    sqlite3_stmt* statement = nullptr;
    if (!mDataPrivate->prepareStatement(updateStatement, &statement))
    {
        DBWRAPPER_LOG_WARN("prepare statement failed, table: " << tableName);
        return -1;
    }

    // Bind SET values
    for (size_t i = 0; i < values.size(); ++i)
    {
        if (!mDataPrivate->bindDBType(statement, values[i], static_cast<int>(i + 1)))
        {
            DBWRAPPER_LOG_WARN("bind SET value failed, table: " << tableName << ", index: " << i);
            sqlite3_finalize(statement);
            return -1;
        }
    }

    // Bind WHERE values
    for (size_t i = 0; i < conditions.size(); ++i)
    {
        const auto& [colName, value, op] = conditions[i];
        if (op != DBOperatorType::IsNull)
        {
            if (!mDataPrivate->bindDBType(statement, value, static_cast<int>(values.size() + i + 1)))
            {
                DBWRAPPER_LOG_WARN("bind WHERE value failed, table: " << tableName << ", index: " << i);
                sqlite3_finalize(statement);
                return -1;
            }
        }
    }

    if (auto result = sqlite3_step(statement); SQLITE_OK != result && SQLITE_DONE != result)
    {
        DBWRAPPER_LOG_WARN("step statement failed, table: " << tableName << ", error code: " << result);
        sqlite3_finalize(statement);
        return -1;
    }

    int64_t changedRows = sqlite3_changes(sqlite3_db_handle(statement));
    
    if (auto result = sqlite3_finalize(statement); SQLITE_OK != result && SQLITE_DONE != result)
    {
        DBWRAPPER_LOG_WARN("finalize statement failed, table: " << tableName << ", error code: " << result);
        return -1;
    }
    
    DBWRAPPER_LOG_DEBUG("update completed, table: " << tableName << ", rows: " << changedRows);
    return changedRows;
}

bool SqliteDatabaseWrapper::updateBatch(const std::string& tableName, const Columns& keyColumns, const Columns& valueColumns, const ListOfArguments& items, const std::source_location location)
{
    DBWRAPPER_LOG_DEBUG("batch update table: " << tableName 
              << ", rows: " << items.size()
              << ", from: " << location.file_name() << '(' << location.line() << ')');

    if (keyColumns.empty() || valueColumns.empty() || items.empty())
    {
        DBWRAPPER_LOG_WARN("batch update with empty data, table: " << tableName);
        return false;
    }

    size_t expectedSize = keyColumns.size() + valueColumns.size();
    for (const auto& item : items)
    {
        if (item.size() != expectedSize)
        {
            DBWRAPPER_LOG_ERROR("mismatch in item size, expected: " << expectedSize << ", got: " << item.size() << ", table: " << tableName);
            return false;
        }
    }

    // Use transaction for atomicity
    if (!beginTransaction())
    {
        DBWRAPPER_LOG_ERROR("failed to begin transaction for batch update, table: " << tableName);
        return false;
    }

    // Build WHERE conditions from key columns
    ListsOfWhereCondition conditions;
    for (const auto& keyCol : keyColumns)
    {
        conditions.emplace_back(keyCol, DatabaseDataValue{}, DBOperatorType::Equal);
    }
    
    std::string updateStatement = generateUpdateStatement(tableName, valueColumns, conditions);
    
    sqlite3_stmt* statement = nullptr;
    if (!mDataPrivate->prepareStatement(updateStatement, &statement))
    {
        DBWRAPPER_LOG_ERROR("prepare statement failed for batch update, table: " << tableName);
        rollback();
        return false;
    }

    bool success = true;
    size_t updatedCount = 0;
    
    for (const auto& item : items)
    {
        // item layout: [keyValues..., valueValues...]
        // Bind order: SET values first (valueColumns), then WHERE values (keyColumns)
        
        // Bind SET values (from valueColumns portion of item)
        for (size_t i = 0; i < valueColumns.size(); ++i)
        {
            size_t itemIndex = keyColumns.size() + i;  // value columns come after key columns in item
            if (!mDataPrivate->bindDBType(statement, item[itemIndex], static_cast<int>(i + 1)))
            {
                DBWRAPPER_LOG_ERROR("bind SET value failed at row " << updatedCount << ", table: " << tableName);
                success = false;
                break;
            }
        }
        
        if (!success) break;
        
        // Bind WHERE values (from keyColumns portion of item)
        for (size_t i = 0; i < keyColumns.size(); ++i)
        {
            size_t paramIndex = valueColumns.size() + i + 1;
            if (!mDataPrivate->bindDBType(statement, item[i], static_cast<int>(paramIndex)))
            {
                DBWRAPPER_LOG_ERROR("bind WHERE value failed at row " << updatedCount << ", table: " << tableName);
                success = false;
                break;
            }
        }
        
        if (!success) break;

        if (auto result = sqlite3_step(statement); SQLITE_OK != result && SQLITE_DONE != result)
        {
            DBWRAPPER_LOG_ERROR("step failed at row " << updatedCount << ", table: " << tableName << ", error: " << result);
            success = false;
            break;
        }

        if (auto result = sqlite3_reset(statement); SQLITE_OK != result)
        {
            DBWRAPPER_LOG_ERROR("reset failed at row " << updatedCount << ", table: " << tableName << ", error: " << result);
            success = false;
            break;
        }
        
        ++updatedCount;
    }

    sqlite3_finalize(statement);

    if (success)
    {
        if (commit())
        {
            DBWRAPPER_LOG_INFO("batch update completed, table: " << tableName << ", rows: " << updatedCount);
            return true;
        }
        else
        {
            DBWRAPPER_LOG_ERROR("commit failed for batch update, table: " << tableName);
            return false;
        }
    }
    else
    {
        DBWRAPPER_LOG_WARN("batch update failed, rolling back, table: " << tableName << ", updated before failure: " << updatedCount);
        rollback();
        return false;
    }
}

int64_t SqliteDatabaseWrapper::deleteFromDatabase(const std::string& tableName, const ListsOfWhereCondition& conditions, const std::source_location location)
{
    DBWRAPPER_LOG_DEBUG("delete from table: " << tableName << ", from: " 
              << location.file_name() << '('
              << location.line() << ':'
              << location.column() << ") `"
              << location.function_name());

    std::string deleteStatement = generateDeleteStatement(tableName, conditions);

    sqlite3_stmt* statement = nullptr;
    if (!mDataPrivate->prepareStatement(deleteStatement, &statement))
    {
        DBWRAPPER_LOG_WARN("prepare statement failed, table: " << tableName);
        return -1;
    }

    // Bind WHERE values
    for (size_t i = 0; i < conditions.size(); ++i)
    {
        const auto& [colName, value, op] = conditions[i];
        if (op != DBOperatorType::IsNull)
        {
            if (!mDataPrivate->bindDBType(statement, value, static_cast<int>(i + 1)))
            {
                DBWRAPPER_LOG_WARN("bind WHERE value failed, table: " << tableName << ", index: " << i);
                sqlite3_finalize(statement);
                return -1;
            }
        }
    }

    if (auto result = sqlite3_step(statement); SQLITE_OK != result && SQLITE_DONE != result)
    {
        DBWRAPPER_LOG_WARN("step statement failed, table: " << tableName << ", error code: " << result);
        sqlite3_finalize(statement);
        return -1;
    }

    int64_t deletedRows = sqlite3_changes(sqlite3_db_handle(statement));
    
    if (auto result = sqlite3_finalize(statement); SQLITE_OK != result && SQLITE_DONE != result)
    {
        DBWRAPPER_LOG_WARN("finalize statement failed, table: " << tableName << ", error code: " << result);
        return -1;
    }
    
    DBWRAPPER_LOG_DEBUG("delete completed, table: " << tableName << ", rows: " << deletedRows);
    return deletedRows;
}

bool SqliteDatabaseWrapper::exists(const std::string& tableName, const ListsOfWhereCondition& conditions)
{
    DBWRAPPER_LOG_DEBUG("check exists in table: " << tableName);
    
    std::stringstream selectStatement;
    selectStatement << "SELECT 1 FROM " << tableName;
    
    if (!conditions.empty())
    {
        selectStatement << " WHERE ";
        for (size_t i = 0; i < conditions.size(); ++i)
        {
            const auto& [colName, value, op] = conditions[i];
            size_t paramIndex = i + 1;
            
            selectStatement << colName;
            switch (op)
            {
                case DBOperatorType::Equal:   selectStatement << " = ?" << paramIndex; break;
                case DBOperatorType::Less:    selectStatement << " < ?" << paramIndex; break;
                case DBOperatorType::Greater: selectStatement << " > ?" << paramIndex; break;
                case DBOperatorType::Not:     selectStatement << " <> ?" << paramIndex; break;
                case DBOperatorType::Like:    selectStatement << " LIKE ?" << paramIndex; break;
                case DBOperatorType::IsNull:  selectStatement << " IS NULL"; break;
                default:                      selectStatement << " = ?" << paramIndex; break;
            }
            
            if (i + 1 < conditions.size())
            {
                selectStatement << " AND ";
            }
        }
    }
    selectStatement << " LIMIT 1";

    sqlite3_stmt* statement = nullptr;
    if (!mDataPrivate->prepareStatement(selectStatement.str(), &statement))
    {
        DBWRAPPER_LOG_WARN("prepare statement failed, table: " << tableName);
        return false;
    }

    // Bind WHERE values
    for (size_t i = 0; i < conditions.size(); ++i)
    {
        const auto& [colName, value, op] = conditions[i];
        if (op != DBOperatorType::IsNull)
        {
            if (!mDataPrivate->bindDBType(statement, value, static_cast<int>(i + 1)))
            {
                DBWRAPPER_LOG_WARN("bind WHERE value failed, table: " << tableName << ", index: " << i);
                sqlite3_finalize(statement);
                return false;
            }
        }
    }

    bool found = (sqlite3_step(statement) == SQLITE_ROW);
    sqlite3_finalize(statement);
    
    DBWRAPPER_LOG_DEBUG("exists check completed, table: " << tableName << ", found: " << found);
    return found;
}

int64_t SqliteDatabaseWrapper::count(const std::string& tableName, const ListsOfWhereCondition& conditions)
{
    DBWRAPPER_LOG_DEBUG("count in table: " << tableName);
    
    std::stringstream selectStatement;
    selectStatement << "SELECT COUNT(*) FROM " << tableName;
    
    if (!conditions.empty())
    {
        selectStatement << " WHERE ";
        for (size_t i = 0; i < conditions.size(); ++i)
        {
            const auto& [colName, value, op] = conditions[i];
            size_t paramIndex = i + 1;
            
            selectStatement << colName;
            switch (op)
            {
                case DBOperatorType::Equal:   selectStatement << " = ?" << paramIndex; break;
                case DBOperatorType::Less:    selectStatement << " < ?" << paramIndex; break;
                case DBOperatorType::Greater: selectStatement << " > ?" << paramIndex; break;
                case DBOperatorType::Not:     selectStatement << " <> ?" << paramIndex; break;
                case DBOperatorType::Like:    selectStatement << " LIKE ?" << paramIndex; break;
                case DBOperatorType::IsNull:  selectStatement << " IS NULL"; break;
                default:                      selectStatement << " = ?" << paramIndex; break;
            }
            
            if (i + 1 < conditions.size())
            {
                selectStatement << " AND ";
            }
        }
    }

    sqlite3_stmt* statement = nullptr;
    if (!mDataPrivate->prepareStatement(selectStatement.str(), &statement))
    {
        DBWRAPPER_LOG_WARN("prepare statement failed, table: " << tableName);
        return -1;
    }

    // Bind WHERE values
    for (size_t i = 0; i < conditions.size(); ++i)
    {
        const auto& [colName, value, op] = conditions[i];
        if (op != DBOperatorType::IsNull)
        {
            if (!mDataPrivate->bindDBType(statement, value, static_cast<int>(i + 1)))
            {
                DBWRAPPER_LOG_WARN("bind WHERE value failed, table: " << tableName << ", index: " << i);
                sqlite3_finalize(statement);
                return -1;
            }
        }
    }

    int64_t rowCount = -1;
    if (sqlite3_step(statement) == SQLITE_ROW)
    {
        rowCount = sqlite3_column_int64(statement, 0);
    }
    
    sqlite3_finalize(statement);
    
    DBWRAPPER_LOG_DEBUG("count completed, table: " << tableName << ", count: " << rowCount);
    return rowCount;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish SqliteDatabaseWrapper Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}