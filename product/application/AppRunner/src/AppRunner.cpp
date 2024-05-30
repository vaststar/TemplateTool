#include "AppRunner/AppRunner.h"
#include <MasterLog/LogExport.h>
#include <ucf/CoreFramework/ICoreFramework.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <ucf/ContactService/IContactService.h>

namespace AppRunner
{
FrameworkDependencies initAppDependencies(const ApplicationConfig& appConfig)
{
    //init logger
	auto fileLogger = std::make_shared<LogLogSpace::LoggerFileConfigure>(appConfig.appLogConfig.logLevel, appConfig.appLogConfig.logDirPath, appConfig.appLogConfig.logBaseFileName, appConfig.appLogConfig.logMaxKeepDays, appConfig.appLogConfig.logMaxSingleFileSize, appConfig.appLogConfig.loggerName);
	MasterLogUtil::InitLogger({fileLogger});

    //create framework
    auto coreFramework = ucf::ICoreFramework::CreateInstance();
    coreFramework->registerService<ucf::IContactService>(ucf::IContactService::CreateInstance(coreFramework));

    coreFramework->initServices();
    
    auto commonheadFramework = commonHead::ICommonHeadFramework::CreateInstance(coreFramework);
    return FrameworkDependencies{coreFramework, commonheadFramework};
}
}