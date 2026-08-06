#pragma once

#include <memory>

#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{

class IFeatureSettingsService;

}

namespace ucf::service::impl{

// Factory entry point for the FeatureSettingsService implementation. Lives in the
// Impl library (FeatureSettingsServiceImpl); only ServiceFactory links against it.
SERVICE_EXPORT std::shared_ptr<ucf::service::IFeatureSettingsService> createFeatureSettingsService(ucf::framework::ICoreFrameworkWPtr coreFramework);

}
