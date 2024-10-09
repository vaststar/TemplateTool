#pragma once

#include <memory>
#include <ucf/DataWarehouse/IDataWarehouse.h>

namespace db{
class DataWarehouse final: public IDataWarehouse
{
public:
    DataWarehouse();
    ~DataWarehouse();
    DataWarehouse(const DataWarehouse&) = delete;
    DataWarehouse(DataWarehouse&&) = delete;
    DataWarehouse& operator=(const DataWarehouse&) = delete;
    DataWarehouse& operator=(DataWarehouse&&) = delete;

    virtual void initializeDB(const DBConfig& dbConfig) override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}