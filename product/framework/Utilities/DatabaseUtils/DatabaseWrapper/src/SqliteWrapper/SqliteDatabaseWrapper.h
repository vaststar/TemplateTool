#pragma once

#include <memory>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/IDatabaseWrapper.h>

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
    virtual void open() override;
    virtual void close() override;
    virtual bool isOpen() override;
    virtual void createTables(const DatabaseSchemas& tableSchemas) override;
    virtual void insertIntoDatabase(const std::string& tableName, const Columns& columns, const ListOfArguments& arguments, const std::source_location location = std::source_location::current()) override;
    virtual void fetchFromDatabase(const std::string& tableName, const ListsOfWhereCondition& arguments, DatabaseDataRecordsCallback func, size_t limit = 0, const std::source_location location = std::source_location::current()) override;
private:
    std::string generateInsertStatement(const std::string& tableName, const Columns& columns) const;
    std::string generateSelectStatement(const std::string& tableName, const Columns& columns, const ListsOfWhereCondition& arguments, size_t limit) const;
    std::string createWhereCondition(const ListsOfWhereCondition& arguments) const;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}