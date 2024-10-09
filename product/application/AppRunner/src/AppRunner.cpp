#include "AppRunner/AppRunner.h"

#include <MasterLog/LogExport.h>
#include <ucf/CoreFramework/ICoreFramework.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <ucf/Services/ContactService/IContactService.h>
#include <ucf/Services/NetworkService/INetworkService.h>

namespace AppRunner
{
FrameworkDependencies initAppDependencies(const ApplicationConfig& appConfig)
{
    //init logger
	auto fileLogger = std::make_shared<LogLogSpace::LoggerFileConfigure>(appConfig.appLogConfig.logLevel, appConfig.appLogConfig.logDirPath, appConfig.appLogConfig.logBaseFileName, appConfig.appLogConfig.logMaxKeepDays, appConfig.appLogConfig.logMaxSingleFileSize, appConfig.appLogConfig.loggerName);
	MasterLogUtil::InitLogger({fileLogger});

    //create framework
    auto coreFramework = ucf::framework::ICoreFramework::CreateInstance();
    coreFramework->initCoreFramework();

    coreFramework->registerService<ucf::service::IContactService>(ucf::service::IContactService::CreateInstance(coreFramework));
    coreFramework->registerService<ucf::service::INetworkService>(ucf::service::INetworkService::CreateInstance(coreFramework));

    coreFramework->initServices();
    
    auto commonheadFramework = commonHead::ICommonHeadFramework::CreateInstance(coreFramework);
    return FrameworkDependencies{coreFramework, commonheadFramework};
}
}