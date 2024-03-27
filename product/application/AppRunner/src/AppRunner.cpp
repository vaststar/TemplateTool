#include "AppRunner/AppRunner.h"
#include "MasterLog/LogExport.h"
#include "CoreFramework/ICoreFramework.h"
#include "CommonHeadFramework/ICommonHeadFramework.h"

namespace AppRunner
{
FrameworkDependencies initAppDependencies(const ApplicationConfig& appConfig)
{
    //init logger
	auto fileLogger = std::make_shared<LogLogSpace::LoggerFileConfigure>(appConfig.appLogConfig.logLevel, appConfig.appLogConfig.logDirPath, appConfig.appLogConfig.logBaseFileName, appConfig.appLogConfig.logMaxKeepDays, appConfig.appLogConfig.logMaxSingleFileSize, "APP");
	MasterLogUtil::InitLogger({fileLogger});

    //create framework
    auto coreFramework = ICoreFramework::CreateInstance();
    auto commonheadFramework = ICommonHeadFramework::CreateInstance(coreFramework);
    return FrameworkDependencies{coreFramework, commonheadFramework};
}
}