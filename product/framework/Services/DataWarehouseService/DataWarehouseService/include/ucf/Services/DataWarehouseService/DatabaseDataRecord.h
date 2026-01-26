#pragma once

#include <string>
#include <map>
#include <ucf/Services/ServiceExportMacro/ServiceExport.h>
#include <ucf/Services/DataWarehouseService/DatabaseDataValue.h>

namespace ucf::service::model{

class DatabaseDataValue;

class SERVICE_EXPORT DatabaseDataRecord final
{
public:
    void addColumnData(const std::string& key, const DatabaseDataValue& value);
    void addColumnData(const std::string& key, DatabaseDataValue&& value);
    DatabaseDataValue getColumnData(const std::string& key) const;
private:
    std::map<std::string, DatabaseDataValue> mValueMaps;
};
}