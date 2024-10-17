#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseDataRecord.h>

#include "DatabaseWrapperLogger.h"
namespace ucf::utilities::database{

void DatabaseDataRecord::addColumnData(const std::string& key, const DatabaseValueStruct& value)
{
    mValueMaps.emplace(key,value);
}

void DatabaseDataRecord::addColumnData(const std::string& key, DatabaseValueStruct&& value)
{
    mValueMaps.emplace(key, std::move(value));
}

}