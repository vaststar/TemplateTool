#include <ucf/Services/DataWarehouseService/DataBaseModel.h>


namespace ucf::service::model{

DBConfig::DBConfig(const std::string& dbId)
    : mDbId(dbId)
{

}

std::string DBConfig::getDBId() const
{
    return mDbId;
}

SqliteDBConfig::SqliteDBConfig(const std::string& dbId, const std::string& filePath, const std::string& password)
    : DBConfig(dbId)
    , mDBFilePath(filePath)
    , mDBPassword(password)
{

}

std::string SqliteDBConfig::getDBFilePath() const
{
    return mDBFilePath;
}

std::string SqliteDBConfig::getDBPassword() const
{
    return mDBPassword;
}

}