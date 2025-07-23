#pragma once

#include <string>

namespace ucf::service{

class IDataWarehouseServiceCallback
{
public:
    virtual ~ IDataWarehouseServiceCallback() = default;
    virtual void OnDatabaseInitialized(const std::string& dbId) = 0;
};
}