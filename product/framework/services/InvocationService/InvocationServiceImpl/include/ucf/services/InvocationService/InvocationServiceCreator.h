#pragma once

#include <memory>

#include <ucf/services/InvocationService/InvocationServiceImplExport.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{

class IInvocationService;

}

namespace ucf::service::impl{

// Factory entry point for the InvocationService implementation. Lives in the
// Impl library (InvocationServiceImpl); only ServiceFactory links against it.
// Kept out of IInvocationService so the Api stays a pure, dependency-free interface.
INVOCATION_SERVICE_IMPL_API std::shared_ptr<ucf::service::IInvocationService> createInvocationService(ucf::framework::ICoreFrameworkWPtr coreFramework);

}
