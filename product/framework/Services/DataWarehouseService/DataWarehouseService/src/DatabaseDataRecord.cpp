
#include <ucf/Services/DataWarehouseService/DatabaseDataRecord.h>

namespace ucf::service::model{

void DatabaseDataRecord::addColumnData(const std::string& key, const DatabaseDataValue& value)
{
    mValueMaps.emplace(key,value);
}

void DatabaseDataRecord::addColumnData(const std::string& key, DatabaseDataValue&& value)
{
    mValueMaps.emplace(key, std::move(value));
}

DatabaseDataValue DatabaseDataRecord::getColumnData(const std::string& key) const
{
    if (auto iter = mValueMaps.find(key); iter != mValueMaps.end())
    {
        return iter->second;
    }
    return DatabaseDataValue("");
}
}