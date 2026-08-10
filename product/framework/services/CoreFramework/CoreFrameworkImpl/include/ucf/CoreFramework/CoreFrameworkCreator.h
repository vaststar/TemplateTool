#pragma once

#include <memory>

#include <ucf/CoreFramework/CoreFrameworkImplExport.h>

namespace ucf::framework{

class ICoreFramework;

// Factory entry point for the CoreFramework implementation. Lives in the Impl
// library (CoreFrameworkImpl); only the application entry point links against it.
CORE_FRAMEWORK_IMPL_API std::shared_ptr<ICoreFramework> createCoreFramework();

}
