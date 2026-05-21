#pragma once

#include <string>
#include <vector>
#include <ucf/Infrastructure/InfrastructureCommonFile/InfrastructureExport.h>
namespace ucf::infrastructure::database{

class Infrastructure_EXPORT DatabaseSchema
{
public:
    struct Column
    {
        std::string mName;
        std::string mAttributes;
    };

    DatabaseSchema(const std::string& tableName, const std::vector<Column>& columns);
    DatabaseSchema(const std::string& tableName, std::vector<Column>&& columns);

    std::string schema() const;
    std::string tableName() const;
    const std::vector<Column>& columns() const;
private:
    std::string mTableName;
    std::vector<Column> mColumns;
};
}
