#pragma once

#include <string>
#include <memory>

#include <ucf/CoreFramework/IService.h>

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
    // virtual void insertIntoDatabase(const std::string& tableName, Columns columns, ListOfArguments strings, bool isVirtual = false, TaskFunction callback = nullptr, TASK_URGENCY urgency = TASK_URGENCY::NORMAL, bool useTransaction = true, const spark::source_location& invocation_location = spark::source_location::current()) = 0;
    static std::shared_ptr<IDataWarehouseService> CreateInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};
}