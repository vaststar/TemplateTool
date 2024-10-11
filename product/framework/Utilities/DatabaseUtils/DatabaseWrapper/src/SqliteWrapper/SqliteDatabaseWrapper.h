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
private:
    std::string generateInsertStatement(const std::string& tableName, const Columns& columns);
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}