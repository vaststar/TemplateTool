#pragma once
#include "AppRunner/AppRunnerExport.h"
#include <memory>
#include <string>

class ICoreFramework;
class ICommonHeadFramework;

namespace AppRunner
{
    struct FrameworkDependencies{
        std::shared_ptr<ICoreFramework> coreFramework;
        std::shared_ptr<ICommonHeadFramework> commonHeadFramework;
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