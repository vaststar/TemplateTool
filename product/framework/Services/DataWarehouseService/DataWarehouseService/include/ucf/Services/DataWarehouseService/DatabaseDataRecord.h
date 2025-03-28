#pragma once

#include <string>
#include <map>
#include <ucf/Services/ServiceExportMacro/ServiceExport.h>
#include <ucf/Services/DataWarehouseService/DataBaseDataValue.h>

namespace ucf::service::model{

class DataBaseDataValue;

class SERVICE_EXPORT DatabaseDataRecord final
{
public:
    void addColumnData(const std::string& key, const DataBaseDataValue& value);
    void addColumnData(const std::string& key, DataBaseDataValue&& value);
    DataBaseDataValue getColumnData(const std::string& key) const;
private:
    std::map<std::string, DataBaseDataValue> mValueMaps;
};
}