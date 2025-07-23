#include "DataWarehouseService.h"

namespace ucf::service{
std::shared_ptr<IDataWarehouseService> IDataWarehouseService::createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework)
{
    return std::make_shared<DataWarehouseService>(coreFramework);
}
}