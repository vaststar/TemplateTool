#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <source_location>

#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseWrapperExport.h>

namespace ucf::utilities::database{

class DatabaseDataRecord;
using DatabaseDataRecords = std::vector<DatabaseDataRecord>;
using DatabaseDataRecordsCallback = std::function<void(const DatabaseDataRecords&)>;

class DatabaseSchema;
using DatabaseSchemas = std::vector<DatabaseSchema>;

using Columns = std::vector<std::string>;
 
class DataBaseDataValue;
using Arguments = std::vector<DataBaseDataValue>;
using ListOfArguments = std::vector<Arguments>;

enum class DBOperatorType
{
    Equal,
    Less,
    Greater,
    Match,
    In,
    NotIn,
    And,
    Like,
    Not,
    IsNull
};
using WhereCondition = std::tuple<std::string, DataBaseDataValue, DBOperatorType>;
using ListsOfWhereCondition = std::vector<WhereCondition>;

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
    virtual void fetchFromDatabase(const std::string& tableName, const ListsOfWhereCondition& arguments, DatabaseDataRecordsCallback func, size_t limit = 0, const std::source_location location = std::source_location::current()) = 0;
       
    static std::shared_ptr<IDatabaseWrapper> createSqliteDatabase(const SqliteDatabaseConfig& config);
};
}