#pragma once

#include <string>
#include <map>
#include <ucf/Services/ServiceCommonFile/ServiceExport.h>
#include <ucf/Services/DataWarehouseService/DataBaseDataValue.h>

namespace ucf::service::model{

class DataBaseDataValue;

class SERVICE_EXPORT DatabaseDataRecord final
{
public:
    void addColumnData(const std::string& key, const DataBaseDataValue& value);
    void addColumnData(const std::string& key, DataBaseDataValue&& value);
private:
    std::map<std::string, DataBaseDataValue> mValueMaps;
};
}