#pragma once

#include <ucf/DataWarehouse/DataWarehouseExport.h>

namespace db{
class DataWarehouse_EXPORT IDataWarehouse
{
public:
    virtual ~IDataWarehouse();
    static std::shared_ptr<IDataWarehouse> createDataWarehouse();

};
}