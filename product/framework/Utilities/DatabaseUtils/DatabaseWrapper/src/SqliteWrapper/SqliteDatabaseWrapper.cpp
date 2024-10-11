#include <sstream>
#include <algorithm>
#include <numeric>

#include <sqlite3.h>

#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseFormatStruct.h>

#include "SqliteWrapper/SqliteDatabaseWrapper.h"
#include "DatabaseWrapperLogger.h"

namespace ucf::utilities::database{
static constexpr auto CREATE_TABLE = "CREATE TABLE IF NOT EXISTS ";
static constexpr auto INSERT_DATA = "INSERT OR REPLACE INTO ";
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
    bool bindDBType(sqlite3_stmt* statement, const DBFormatStruct& value, int index);
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
        std::string errorMessage = sqlError;
        sqlite3_free(sqlError);
        DBWRAPPER_LOG_WARN("execute db command failed, dbname: " << mDatabaseConfig.fileName << ", error: " << errorMessage);
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

bool SqliteDatabaseWrapper::DataPrivate::bindDBType(sqlite3_stmt* statement, const DBFormatStruct& value, int index)
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
        std::string createStatement = CREATE_TABLE + tableInfo.tableName() + tableInfo.schema();
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

    for(const auto& argument: arguments)
    {
        for(size_t i = 0; i < argument.size(); ++i)
        {
            mDataPrivate->bindDBType(statement, argument[i], i+1);
        }
        if (auto result = sqlite3_step(statement); SQLITE_OK != result && SQLITE_DONE != result)
        {
            DBWRAPPER_LOG_WARN("step statment failed, table: " << tableName << ", error code: " << result);
            return;
        }
        if (auto result = sqlite3_reset(statement); SQLITE_OK != result && SQLITE_DONE != result)
        {
            DBWRAPPER_LOG_WARN("reset statment failed, table: " << tableName << ", error code: " << result);
            return;
        }
    }
    if (auto result = sqlite3_finalize(statement); SQLITE_OK != result && SQLITE_DONE != result)
    {
        DBWRAPPER_LOG_WARN("finalize statment failed, table: " << tableName << ", error code: " << result);
        return;
    }
}

std::string SqliteDatabaseWrapper::generateInsertStatement(const std::string& tableName, const Columns& columns)
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

    insertStatement << INSERT_DATA << tableName << fieldsStatement << "VALUES" << columnPlaceholderStatement;
    return insertStatement.str();
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish SqliteDatabaseWrapper Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}