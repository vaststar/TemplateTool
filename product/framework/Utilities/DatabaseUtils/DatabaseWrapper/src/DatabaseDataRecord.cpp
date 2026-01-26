#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseDataRecord.h>

#include "DatabaseWrapperLogger.h"
namespace ucf::utilities::database{

void DatabaseDataRecord::addColumnData(const std::string& key, const DatabaseDataValue& value)
{
    mValueMaps.emplace(key,value);
}

void DatabaseDataRecord::addColumnData(const std::string& key, DatabaseDataValue&& value)
{
    mValueMaps.emplace(key, std::move(value));
}

const std::map<std::string, DatabaseDataValue>& DatabaseDataRecord::getData() const
{
    return mValueMaps;
}
}