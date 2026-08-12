#pragma once

#include <memory>

#include <ucf/services/DataWarehouseService/DataWarehouseServiceImplExport.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{

class IDataWarehouseService;

}

namespace ucf::service::impl{

// Factory entry point for the DataWarehouseService implementation. Lives in the
// Impl library (DataWarehouseServiceImpl); only ServiceFactory links against it.
// Kept out of IDataWarehouseService so the Api stays a pure, dependency-free interface.
DATA_WAREHOUSE_SERVICE_IMPL_API std::shared_ptr<ucf::service::IDataWarehouseService> createDataWarehouseService(ucf::framework::ICoreFrameworkWPtr coreFramework);

}
