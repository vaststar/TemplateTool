#pragma once

#include <memory>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/IDatabaseWrapper.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseConfig.h>

namespace ucf::utilities::database{

class SqliteDatabaseWrapper final: public IDatabaseWrapper
{
public:
    explicit SqliteDatabaseWrapper(const SqliteDatabaseConfig& config);
    ~SqliteDatabaseWrapper();
    SqliteDatabaseWrapper(const SqliteDatabaseWrapper&) = delete;
    SqliteDatabaseWrapper(SqliteDatabaseWrapper&&) = delete;
    SqliteDatabaseWrapper& operator=(const SqliteDatabaseWrapper&) = delete;
    SqliteDatabaseWrapper& operator=(SqliteDatabaseWrapper&&) = delete;

public:
    bool open() override;
    void close() override;
    bool isOpen() override;
    void createTables(const DatabaseSchemas& tableSchemas) override;
    bool insertIntoDatabase(const std::string& tableName, const Columns& columns, const Arguments& arguments, const std::source_location location = std::source_location::current()) override;
    bool insertBatch(const std::string& tableName, const Columns& columns, const ListOfArguments& arguments, const std::source_location location = std::source_location::current()) override;
    void fetchFromDatabase(const std::string& tableName, const Columns& columns, const ListsOfWhereCondition& arguments, DatabaseDataRecordsCallback func, size_t limit = 0, const std::source_location location = std::source_location::current()) override;
    int64_t updateInDatabase(const std::string& tableName, const Columns& columns, const Arguments& values, const ListsOfWhereCondition& conditions, const std::source_location location = std::source_location::current()) override;
    bool updateBatch(const std::string& tableName, const Columns& keyColumns, const Columns& valueColumns, const ListOfArguments& items, const std::source_location location = std::source_location::current()) override;
    int64_t deleteFromDatabase(const std::string& tableName, const ListsOfWhereCondition& conditions, const std::source_location location = std::source_location::current()) override;
    bool exists(const std::string& tableName, const ListsOfWhereCondition& conditions) override;
    int64_t count(const std::string& tableName, const ListsOfWhereCondition& conditions) override;

private:
    bool beginTransaction();
    bool commit();
    bool rollback();
    
    std::string generateInsertStatement(const std::string& tableName, const Columns& columns) const;
    std::string generateSelectStatement(const std::string& tableName, const Columns& columns, const ListsOfWhereCondition& arguments, size_t limit) const;
    std::string generateUpdateStatement(const std::string& tableName, const Columns& columns, const ListsOfWhereCondition& conditions) const;
    std::string generateDeleteStatement(const std::string& tableName, const ListsOfWhereCondition& conditions) const;
    std::string createWhereCondition(const ListsOfWhereCondition& arguments) const;

private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}