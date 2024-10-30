#pragma once

#include <string>
#include <ucf/Services/ServiceExportMacro/ServiceExport.h>
namespace ucf::service::model{

class SERVICE_EXPORT DBConfig
{
public:
    explicit DBConfig(const std::string& dbId);
    virtual ~DBConfig() = default;

    std::string getDBId() const;
private:
    std::string mDbId;
};

class SERVICE_EXPORT SqliteDBConfig: public DBConfig
{
public:
    SqliteDBConfig(const std::string& dbId, const std::string& filePath, const std::string& password = std::string{});

    std::string getDBFilePath() const;
    std::string getDBPassword() const;
private:
    std::string mDBFilePath;
    std::string mDBPassword;

};
}