#pragma once

#include <string>
#include <memory>
#include <vector>

#include <ucf/CoreFramework/IService.h>
#include <ucf/Services/DataWarehouseService/DataBaseModel.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{

class SERVICE_EXPORT IDataWarehouseService: public IService
{
public:
    virtual void initializeDB(std::shared_ptr<model::DBConfig> dbConfig, const std::vector<model::DBTableModel>& tables = {}) = 0;
    virtual void insertIntoDatabase(const std::string& dbId, const std::string& tableName, const model::DBColumnFields& columnFields, const model::ListOfDBValues& values) = 0;
    static std::shared_ptr<IDataWarehouseService> CreateInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}