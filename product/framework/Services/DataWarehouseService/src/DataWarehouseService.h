#pragma once

#include <memory>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>

namespace ucf::service{
class DataWarehouseService final: public IDataWarehouseService
{
public:
    explicit DataWarehouseService(ucf::framework::ICoreFrameworkWPtr coreFramework);
    virtual ~DataWarehouseService();
    DataWarehouseService(const DataWarehouseService&) = delete;
    DataWarehouseService(DataWarehouseService&&) = delete;
    DataWarehouseService& operator=(const DataWarehouseService&) = delete;
    DataWarehouseService& operator=(DataWarehouseService&&) = delete;

    //IService
    virtual std::string getServiceName() const override;
    virtual void initService() override;
    //IDataWarehouseService
    virtual void initializeDB(const model::DBConfig& dbConfig) override;
private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};
}