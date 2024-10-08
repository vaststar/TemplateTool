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
    virtual void execute(const std::string& commandStr) override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}