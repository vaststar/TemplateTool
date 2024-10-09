#include <algorithm>
#include <numeric>
#include <iterator>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DataBaseSchema.h>

namespace ucf::utilities::database{
DataBaseSchema::DataBaseSchema(const std::string& tableName, const std::vector<Column>& columns)
    : mTableName(tableName)
    , mColumns(columns)
{
}
DataBaseSchema::DataBaseSchema(const std::string& tableName, std::vector<Column>&& columns)
    : mTableName(tableName)
{
    mColumns.swap(columns);
}

std::string DataBaseSchema::schema() const
{
    if (mColumns.empty())
    {
        return {};
    }
    std::vector<std::string> fields;
    fields.reserve(mColumns.size());
    std::transform(mColumns.cbegin(), mColumns.cend(), std::back_inserter(fields), [](const Column& field) {
        return field.mName + " " + field.mAttributes;
    });
    return std::string("(") + std::accumulate(std::next(fields.begin()), fields.end(), fields.front(), [](std::string a, std::string b){return a+","+b;}) + ")";
}

std::string DataBaseSchema::tableName() const
{
    return mTableName;
}
}