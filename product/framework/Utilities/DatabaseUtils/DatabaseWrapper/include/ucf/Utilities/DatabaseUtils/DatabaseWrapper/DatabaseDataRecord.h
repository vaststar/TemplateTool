#pragma once

#include <string>
#include <map>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DatabaseWrapperExport.h>
#include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/DataBaseDataValue.h>

namespace ucf::utilities::database{

class DataBaseDataValue;

class DATABASEWRAPPER_EXPORT DatabaseDataRecord final
{
public:
    void addColumnData(const std::string& key, const DataBaseDataValue& value);
    void addColumnData(const std::string& key, DataBaseDataValue&& value);
    const std::map<std::string, DataBaseDataValue>& getData() const;
private:
    std::map<std::string, DataBaseDataValue> mValueMaps;
};
}