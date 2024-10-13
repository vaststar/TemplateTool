#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <source_location>

#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseWrapperExport.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseSchema.h>

namespace ucf::utilities::database{

class DatabaseQueryResult;
using DatabaseQueryResults = std::vector<DatabaseQueryResult>;

class DatabaseSchema;
using DatabaseSchemas = std::vector<DatabaseSchema>;

using Columns = std::vector<std::string>; 
class DBFormatStruct;
using Arguments = std::vector<DBFormatStruct>;
using ListOfArguments = std::vector<Arguments>;

struct SqliteDatabaseConfig{
    std::string fileName;
    std::string password;
};

class DATABASEWRAPPER_EXPORT IDatabaseWrapper
{
public:
    virtual ~IDatabaseWrapper() = default;
public:
    virtual void open() = 0;
    virtual void close() = 0;
    virtual bool isOpen() = 0;

    virtual void createTables(const DatabaseSchemas& tableSchemas) = 0;
    virtual void insertIntoDatabase(const std::string& tableName, const Columns& columns, const ListOfArguments& arguments, const std::source_location location = std::source_location::current()) = 0;
    // virtual void queryData(const std::string& queryStr, std::function<void(const DatabaseQueryResults&)>) = 0;

    static std::shared_ptr<IDatabaseWrapper> createSqliteDatabase(const SqliteDatabaseConfig& config);
};
}