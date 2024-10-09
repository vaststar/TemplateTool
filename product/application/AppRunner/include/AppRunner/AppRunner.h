#pragma once
#include "AppRunner/AppRunnerExport.h"
#include <memory>
#include <string>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkPtr = std::shared_ptr<ICoreFramework>;
}

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkPtr = std::shared_ptr<ICommonHeadFramework>;
}

namespace AppRunner
{
    struct FrameworkDependencies{
        ucf::framework::ICoreFrameworkPtr coreFramework;
        commonHead::ICommonHeadFrameworkPtr commonHeadFramework;
    };
    struct AppLogConfig{
        std::string logDirPath;
	    std::string logBaseFileName;
        int logLevel{ 0 };
        unsigned int logMaxKeepDays{ 180 };
        unsigned int logMaxSingleFileSize{ 50 * 1024 * 1024};
        std::string loggerName{"APP"};//don't change it, we all use APP as loggerName
    };
    
    struct ApplicationConfig{
        AppLogConfig appLogConfig;
    };
    APPRUNNER_EXPORT [[nodiscard]] FrameworkDependencies initAppDependencies(const ApplicationConfig& appConfig);
}