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

DBTableModel::DBTableModel(const std::string& tableName, const std::vector<Column>& columns)
    : mTableName(tableName)
    , mColumns(columns)
{
}

DBTableModel::DBTableModel(const std::string& tableName, std::vector<Column>&& columns)
    : mTableName(tableName)
{
    mColumns.swap(columns);
}

std::string DBTableModel::tableName() const
{
    return mTableName;
}

const std::vector<DBTableModel::Column>& DBTableModel::columns() const
{
    return mColumns;
}
}