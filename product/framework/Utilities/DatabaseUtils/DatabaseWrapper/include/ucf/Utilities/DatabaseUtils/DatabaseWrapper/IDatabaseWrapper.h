#pragma once

#include <memory>
#include <string>
#include <functional>

#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseWrapperExport.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseSchema.h>

namespace ucf::utilities::database{

class DatabaseQueryResult;
using DatabaseQueryResults = std::vector<DatabaseQueryResult>;

class DatabaseSchema;
using DatabaseSchemas = std::vector<DatabaseSchema>;

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
    // virtual void queryData(const std::string& queryStr, std::function<void(const DatabaseQueryResults&)>) = 0;

    static std::shared_ptr<IDatabaseWrapper> createSqliteDatabase(const SqliteDatabaseConfig& config);
};
}