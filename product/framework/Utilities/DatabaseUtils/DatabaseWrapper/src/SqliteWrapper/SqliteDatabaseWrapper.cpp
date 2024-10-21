#include <sstream>
#include <algorithm>
#include <numeric>

#include <sqlite3.h>

#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DataBaseDataValue.h>
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
    void openDatabase();
    void closeDatabase();
    bool isOpen();
    void execute(const std::string& commandStr);
    bool prepareStatement(const std::string& statement, sqlite3_stmt** ppStmt);
    bool bindDBType(sqlite3_stmt* statement, const DataBaseDataValue& value, int index);
    void extractResultsFromStatement(sqlite3_stmt* statement, DatabaseDataRecords& result);
private:
    SqliteDatabaseConfig mDatabaseConfig;
    sqlite3* mDatabase;
};

SqliteDatabaseWrapper::DataPrivate::DataPrivate(const SqliteDatabaseConfig& config)
    : mDatabaseConfig(config)
    , mDatabase(nullptr)
{
}

void SqliteDatabaseWrapper::DataPrivate::openDatabase()
{
   if (SQLITE_OK == sqlite3_open(mDatabaseConfig.fileName.c_str(), &mDatabase))
   {
        DBWRAPPER_LOG_INFO("open db successfully, dbname: " << mDatabaseConfig.fileName);
        // if (!mDatabaseConfig.password.empty() && 
        //     SQLITE_OK == sqlite3_key(mDatabase, mDatabaseConfig.password.c_str(), mDatabaseConfig.password.length()))
        // {
        //     DBWRAPPER_LOG_INFO("open db successfully, dbname: " << mDatabaseConfig.fileName);
        //     closeDatabase();
        // }
        // else
        // {
        //     DBWRAPPER_LOG_WARN("db password wrong, dbname: " << mDatabaseConfig.fileName);
        // }
   }
   else
   {
        DBWRAPPER_LOG_WARN("open db failed, dbname: " << mDatabaseConfig.fileName);
   }
}

void SqliteDatabaseWrapper::DataPrivate::closeDatabase()
{
    if (mDatabase)
    {
        sqlite3_close(mDatabase);
        mDatabase = nullptr;
    }
}

bool SqliteDatabaseWrapper::DataPrivate::isOpen()
{
    return nullptr != mDatabase;
}

void SqliteDatabaseWrapper::DataPrivate::execute(const std::string& commandStr)
{
    char* sqlError = nullptr;
    if (auto result = sqlite3_exec(mDatabase, commandStr.c_str(), nullptr, nullptr, &sqlError); SQLITE_OK != result)
    {
        std::string errorMessage;
        if (sqlError)
        {
            errorMessage = sqlError;
            sqlite3_free(sqlError);
        }
        DBWRAPPER_LOG_WARN("execute db command failed, dbname: " << mDatabaseConfig.fileName << ", result code: " << result << ", error: " << errorMessage);
    }
}

bool SqliteDatabaseWrapper::DataPrivate::prepareStatement(const std::string& statement, sqlite3_stmt** ppStmt)
{
    if (auto result = sqlite3_prepare_v2(mDatabase, statement.c_str(), static_cast<int>(statement.size()), ppStmt, nullptr); SQLITE_OK != result)
    {
        DBWRAPPER_LOG_WARN("prepare statement failed, error code: " << result);
        return false;
    }
    return true;
}

bool SqliteDatabaseWrapper::DataPrivate::bindDBType(sqlite3_stmt* statement, const DataBaseDataValue& value, int index)
{
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
            if (auto result = sqlite3_bind_blob(statement, index, "", 0, SQLITE_STATIC); SQLITE_OK == result)
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
            if (auto result = sqlite3_bind_blob(statement, index, &bufferData[0], static_cast<int>(bufferData.size()), SQLITE_STATIC); SQLITE_OK == result)
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
                oneRowResult.addColumnData(columnName, "");
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
std::shared_ptr<IDatabaseWrapper> IDatabaseWrapper::createSqliteDatabase(const SqliteDatabaseConfig& config)
{
    return std::make_shared<SqliteDatabaseWrapper>(config);
}

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
    catch(...)
    {
        DBWRAPPER_LOG_WARN("close db exception"); 
    }
}

void SqliteDatabaseWrapper::open()
{
    mDataPrivate->openDatabase();
}

void SqliteDatabaseWrapper::close()
{
    mDataPrivate->closeDatabase();
}

bool SqliteDatabaseWrapper::isOpen()
{
    return mDataPrivate->isOpen();
}

void SqliteDatabaseWrapper::createTables(const DatabaseSchemas& tableSchemas)
{
    for (const auto& tableInfo : tableSchemas)
    {
        std::string createStatement = "CREATE TABLE IF NOT EXISTS " + tableInfo.tableName() + tableInfo.schema();
        mDataPrivate->execute(createStatement);
    }
}

void SqliteDatabaseWrapper::insertIntoDatabase(const std::string& tableName, const Columns& columns, const ListOfArguments& arguments, const std::source_location location)
{
    DBWRAPPER_LOG_DEBUG("about to insert data into table: " << tableName << ", from: " 
              << location.file_name() << '('
              << location.line() << ':'
              << location.column() << ") `"
              << location.function_name());

    if (columns.empty() || arguments.empty())
    {
        DBWRAPPER_LOG_WARN("insert empty data into table: " << tableName);
        return;
    }

    for (const auto& argument: arguments)
    {
        if (columns.size() != argument.size())
        {
            DBWRAPPER_LOG_WARN("mismatch in columns and data, table: " << tableName);
            return;
        }
    }
    
    std::string insertStatement = generateInsertStatement(tableName, columns);

    sqlite3_stmt* statement = nullptr;
    if (!mDataPrivate->prepareStatement(insertStatement, &statement))
    {
        DBWRAPPER_LOG_WARN("prepare statment failed, table: " << tableName);
        return;
    }

    bool errorHappened = false;
    for(const auto& argument: arguments)
    {
        for(size_t i = 0; i < argument.size(); ++i)
        {
            if (!mDataPrivate->bindDBType(statement, argument[i], i+1))
            {
                DBWRAPPER_LOG_WARN("bind statment failed, table: " << tableName);
                errorHappened = true;
                break;
            }
        }
        if (errorHappened)
        {
            break;
        }

        if (auto result = sqlite3_step(statement); SQLITE_OK != result && SQLITE_DONE != result)
        {
            DBWRAPPER_LOG_WARN("step statment failed, table: " << tableName << ", error code: " << result);
            break;
        }

        if (auto result = sqlite3_reset(statement); SQLITE_OK != result && SQLITE_DONE != result)
        {
            DBWRAPPER_LOG_WARN("reset statment failed, table: " << tableName << ", error code: " << result);
            break;
        }
    }
    if (auto result = sqlite3_finalize(statement); SQLITE_OK != result && SQLITE_DONE != result)
    {
        DBWRAPPER_LOG_WARN("finalize statment failed, table: " << tableName << ", error code: " << result);
        return;
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
            it->second.push_back({ arguments[i], i + 1 });
        }
        else
        {
            groupedWhereConditions[name] = { { arguments[i], i + 1 } };
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

void SqliteDatabaseWrapper::fetchFromDatabase(const std::string& tableName, const ListsOfWhereCondition& arguments, DatabaseDataRecordsCallback func, size_t limit, const std::source_location location)
{
    DBWRAPPER_LOG_DEBUG("fetch data from table: " << tableName << ", from: " 
              << location.file_name() << '('
              << location.line() << ':'
              << location.column() << ") `"
              << location.function_name());
    
    std::string selectStatement = generateSelectStatement(tableName, Columns{{"*"}}, arguments, limit);
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

    DBWRAPPER_LOG_WARN("fetch data succeed, table: " << tableName);
    func(result);
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish SqliteDatabaseWrapper Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}