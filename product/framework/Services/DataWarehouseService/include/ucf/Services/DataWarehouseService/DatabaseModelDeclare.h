#pragma once

#include <string>
#include <vector> 
#include <functional>
namespace ucf::service::model{
class DBConfig;
class DBTableModel;

class DataBaseDataValue;
using DBDataValues = std::vector<DataBaseDataValue>;
using ListOfDBValues = std::vector<DBDataValues>;    
using DBColumnFields = std::vector<std::string>;

class DatabaseDataRecord;
using DatabaseDataRecords = std::vector<DatabaseDataRecord>;
using DatabaseDataRecordsCallback = std::function<void(const DatabaseDataRecords&)>;

enum class DBOperatorType;
using WhereCondition = std::tuple<std::string, DataBaseDataValue, DBOperatorType>;
using ListsOfWhereCondition = std::vector<WhereCondition>;


enum class DBOperatorType
{
    Equal,
    Less,
    Greater,
    Match,
    In,
    NotIn,
    And,
    Like,
    Not,
    IsNull
};
}