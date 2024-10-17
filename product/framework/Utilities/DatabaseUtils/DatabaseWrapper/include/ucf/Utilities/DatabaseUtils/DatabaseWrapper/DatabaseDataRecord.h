#pragma once

#include <string>
#include <map>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseWrapperExport.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseValueStruct.h>

namespace ucf::utilities::database{

class DatabaseValueStruct;

class DATABASEWRAPPER_EXPORT DatabaseDataRecord final
{
public:
    void addColumnData(const std::string& key, const DatabaseValueStruct& value);
    void addColumnData(const std::string& key, DatabaseValueStruct&& value);
private:
    std::map<std::string, DatabaseValueStruct> mValueMaps;
};
}