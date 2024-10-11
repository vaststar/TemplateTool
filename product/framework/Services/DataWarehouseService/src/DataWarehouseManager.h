#pragma once

#include <memory>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{
    
namespace model{
    struct DBConfig;
}

class DataWarehouseManager final
{
public:
    explicit DataWarehouseManager(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~DataWarehouseManager();
    DataWarehouseManager(const DataWarehouseManager&) = delete;
    DataWarehouseManager(DataWarehouseManager&&) = delete;
    DataWarehouseManager& operator=(const DataWarehouseManager&) = delete;
    DataWarehouseManager& operator=(DataWarehouseManager&&) = delete;
public:
    void initializeDB(const model::DBConfig& dbConfig);
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}