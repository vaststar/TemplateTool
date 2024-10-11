#include <sqlite3.h>

#include "SqliteWrapper/SqliteDatabaseWrapper.h"
#include "DatabaseWrapperLogger.h"

namespace ucf::utilities::database{
static constexpr auto CREATE_TABLE = "CREATE TABLE IF NOT EXISTS ";
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
    if (SQLITE_OK != sqlite3_exec(mDatabase, commandStr.c_str(), nullptr, nullptr, &sqlError))
    {
        std::string errorMessage = sqlError;
        sqlite3_free(sqlError);
        DBWRAPPER_LOG_WARN("execute db command failed, dbname: " << mDatabaseConfig.fileName << ", error: " << errorMessage);
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
        std::string createStatement = CREATE_TABLE + tableInfo.tableName() + tableInfo.schema();
        mDataPrivate->execute(createStatement);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish SqliteDatabaseWrapper Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}