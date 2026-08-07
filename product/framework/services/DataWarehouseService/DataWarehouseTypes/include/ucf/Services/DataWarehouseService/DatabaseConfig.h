#pragma once

#include <string>
#include <ucf/Services/DataWarehouseService/DataWarehouseTypesExport.h>
namespace ucf::service::model{

class DATA_WAREHOUSE_TYPES_API DBConfig
{
public:
    DBConfig();
    explicit DBConfig(const std::string& dbId);
    virtual ~DBConfig() = default;

    std::string getDBId() const;
private:
    std::string mDbId;
};

class DATA_WAREHOUSE_TYPES_API SqliteDBConfig: public DBConfig
{
public:
    SqliteDBConfig();
    SqliteDBConfig(const std::string& dbId, const std::string& filePath, const std::string& password = std::string{});

    std::string getDBFilePath() const;
    std::string getDBPassword() const;
private:
    std::string mDBFilePath;
    std::string mDBPassword;

};
}
