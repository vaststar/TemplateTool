
#include <ucf/Services/DataWarehouseService/DatabaseDataRecord.h>

namespace ucf::service::model{

void DatabaseDataRecord::addColumnData(const std::string& key, const DataBaseDataValue& value)
{
    mValueMaps.emplace(key,value);
}

void DatabaseDataRecord::addColumnData(const std::string& key, DataBaseDataValue&& value)
{
    mValueMaps.emplace(key, std::move(value));
}

}