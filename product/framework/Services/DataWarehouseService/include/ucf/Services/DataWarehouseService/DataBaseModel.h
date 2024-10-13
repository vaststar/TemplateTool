#pragma once

#include <string>
#include <vector>
#include <variant>
#include <ucf/Services/ServiceCommonFile/ServiceExport.h>
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

namespace DBDataValueType
{
    using BOOL = bool;
    using STRING = std::string;
    using INT = int;
    using LONG = long;
    using LONGLONG = long long;
    using FLOAT = float;
    using BUFFER = std::vector<uint8_t>;
} // namespace DBDataValueType

using DBDataValue = std::variant<DBDataValueType::BOOL, DBDataValueType::LONG, DBDataValueType::LONGLONG, DBDataValueType::STRING, DBDataValueType::INT, DBDataValueType::FLOAT, DBDataValueType::BUFFER>;
using DBDataValues = std::vector<DBDataValue>;
using DBColumnFields = std::vector<std::string>;

class SERVICE_EXPORT DBTableModel
{
public:
    struct Column
    {
        std::string mName;
        std::string mAttributes;
    };

    DBTableModel(const std::string& tableName, const std::vector<Column>& columns);
    DBTableModel(const std::string& tableName, std::vector<Column>&& columns);

    std::string tableName() const;
    const std::vector<Column>& columns() const;
private:
    std::string mTableName;
    std::vector<Column> mColumns;
};

}