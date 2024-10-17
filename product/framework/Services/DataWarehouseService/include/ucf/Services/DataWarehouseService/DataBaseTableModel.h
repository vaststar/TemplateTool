#pragma once

#include <string>
#include <vector>
#include <variant>
#include <ucf/Services/ServiceCommonFile/ServiceExport.h>
namespace ucf::service::model{
//for DBTable
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