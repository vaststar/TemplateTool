#pragma once

#include <memory>

#include <commonhead/ServiceLocator/IServiceLocator.h>
#include <commonhead/ServiceLocator/ServiceLocatorImplExport.h>

namespace ucf::framework{
class ICoreFramework;
}

namespace commonHead::impl{

SERVICE_LOCATOR_IMPL_API std::shared_ptr<IServiceLocator> createServiceLocator(
    std::weak_ptr<ucf::framework::ICoreFramework> coreFramework);

}
