#pragma once

#include <string>
#include <memory>

#include <ucf/CoreFramework/IService.h>
#include <ucf/Services/DataWarehouseService/DataBaseModel.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{

namespace model{
    struct DBConfig;
}

class SERVICE_EXPORT IDataWarehouseService: public IService
{
public:
    virtual void initializeDB(const model::DBConfig& dbConfig) = 0;
    static std::shared_ptr<IDataWarehouseService> CreateInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}