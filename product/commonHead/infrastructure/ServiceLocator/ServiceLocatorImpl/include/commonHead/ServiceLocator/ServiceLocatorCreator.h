#pragma once

#include <memory>

#include <commonHead/ServiceLocator/IServiceLocator.h>
#include <commonHead/ServiceLocator/ServiceLocatorImplExport.h>

namespace ucf::framework{
class ICoreFramework;
}

namespace commonHead::impl{

SERVICE_LOCATOR_IMPL_API std::shared_ptr<IServiceLocator> createServiceLocator(
    std::weak_ptr<ucf::framework::ICoreFramework> coreFramework);

}
