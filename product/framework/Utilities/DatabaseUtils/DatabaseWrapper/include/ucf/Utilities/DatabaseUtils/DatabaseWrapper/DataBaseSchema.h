#pragma once

#include <string>
#include <vector>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseWrapperExport.h>
namespace ucf::utilities::database{

class DATABASEWRAPPER_EXPORT DataBaseSchema
{
public:
    struct Column
    {
        std::string mName;
        std::string mAttributes;
    };

    DataBaseSchema(const std::string& tableName, const std::vector<Column>& columns);
    DataBaseSchema(const std::string& tableName, std::vector<Column>&& columns);

    std::string schema() const;
    std::string tableName() const;
private:
    std::string mTableName;
    std::vector<Column> mColumns;
};
}