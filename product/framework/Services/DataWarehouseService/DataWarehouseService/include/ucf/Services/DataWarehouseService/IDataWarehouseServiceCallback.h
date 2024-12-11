#pragma once

#include <string>
#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::service{

class SERVICE_EXPORT IDataWarehouseServiceCallback
{
public:
    virtual ~ IDataWarehouseServiceCallback() = default;
    virtual void OnDatabaseInitialized(const std::string& dbId) = 0;
};
}