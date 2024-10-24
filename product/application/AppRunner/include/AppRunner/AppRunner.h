#pragma once
#include "AppRunner/AppRunnerExport.h"
#include <memory>
#include <string>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkPtr = std::shared_ptr<ICoreFramework>;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkPtr = std::shared_ptr<ICommonHeadFramework>;
    using ICommonHeadFrameworkWPtr = std::weak_ptr<ICommonHeadFramework>;
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

    class APPRUNNER_EXPORT ApplicationRunner final
    {
    public:
        ApplicationRunner();
        ~ApplicationRunner();
    public:
        void createApp(int argc, char* argv[]);
        void initApp();
        void exitApp();
        commonHead::ICommonHeadFrameworkWPtr getCommonheadFramework() const;
        ucf::framework::ICoreFrameworkWPtr getCoreFramework() const;

    private:
        class DataPrivate;
        std::unique_ptr<DataPrivate> mDataPrivate;
    };
    // APPRUNNER_EXPORT [[nodiscard]] FrameworkDependencies initAppDependencies(const ApplicationConfig& appConfig);
}