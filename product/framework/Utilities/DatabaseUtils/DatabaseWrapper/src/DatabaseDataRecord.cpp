#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseDataRecord.h>

#include "DatabaseWrapperLogger.h"
namespace ucf::utilities::database{

void DatabaseDataRecord::addColumnData(const std::string& key, const DataBaseDataValue& value)
{
    mValueMaps.emplace(key,value);
}

void DatabaseDataRecord::addColumnData(const std::string& key, DataBaseDataValue&& value)
{
    mValueMaps.emplace(key, std::move(value));
}

const std::map<std::string, DataBaseDataValue>& DatabaseDataRecord::getData() const
{
    return mValueMaps;
}
}