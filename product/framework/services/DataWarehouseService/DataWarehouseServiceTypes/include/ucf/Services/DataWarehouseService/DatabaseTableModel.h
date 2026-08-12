#pragma once

#include <string>
#include <vector>
#include <variant>
#include <ucf/Services/DataWarehouseService/DataWarehouseServiceTypesExport.h>
namespace ucf::service::model{
//for DBTable
class DATA_WAREHOUSE_SERVICE_TYPES_API DBTableModel
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
