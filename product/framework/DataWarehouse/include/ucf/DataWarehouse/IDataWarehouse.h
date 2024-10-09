#pragma once

#include <string>
#include <ucf/DataWarehouse/DataWarehouseExport.h>

namespace db{

enum class DBEnum{
    SHARED_DB,
    USER_DB
};
struct DBConfig{
    DBEnum dbType{DBEnum::SHARED_DB};
    std::string dbFilePath;
    std::string password;
};

class DataWarehouse_EXPORT IDataWarehouse
{
public:
    virtual ~IDataWarehouse() = default;
    virtual void initializeDB(const DBConfig& dbConfig) = 0;
    static std::shared_ptr<IDataWarehouse> createDataWarehouse();

};
}