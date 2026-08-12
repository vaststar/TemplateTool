#pragma once

#include <memory>

#include <ucf/services/ContactService/ContactServiceImplExport.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{

class IContactService;

}

namespace ucf::service::impl{

// Factory entry point for the ContactService implementation. Lives in the
// Impl library (ContactServiceImpl); only ServiceFactory links against it.
// Kept out of IContactService so the Api stays a pure, dependency-free interface.
CONTACT_SERVICE_IMPL_API std::shared_ptr<ucf::service::IContactService> createContactService(ucf::framework::ICoreFrameworkWPtr coreFramework);

}
