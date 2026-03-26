#pragma once

#include <string>

namespace ucf::service{

class IDataWarehouseServiceCallback
{
public:
    IDataWarehouseServiceCallback() = default;
    IDataWarehouseServiceCallback(const IDataWarehouseServiceCallback&) = delete;
    IDataWarehouseServiceCallback(IDataWarehouseServiceCallback&&) = delete;
    IDataWarehouseServiceCallback& operator=(const IDataWarehouseServiceCallback&) = delete;
    IDataWarehouseServiceCallback& operator=(IDataWarehouseServiceCallback&&) = delete;
    virtual ~ IDataWarehouseServiceCallback() = default;
public:
    virtual void OnDatabaseInitialized(const std::string& dbId) = 0;
};
}