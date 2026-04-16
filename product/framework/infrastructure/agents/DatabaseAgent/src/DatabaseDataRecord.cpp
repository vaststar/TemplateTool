#include <ucf/Agents/DatabaseAgent/DatabaseDataRecord.h>

#include "DatabaseAgentLogger.h"
namespace ucf::agents::database{

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
