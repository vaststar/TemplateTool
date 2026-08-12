#include <ucf/Services/DataWarehouseService/DatabaseTableModel.h>

namespace ucf::service::model{
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