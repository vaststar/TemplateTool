#include <optional>
#include <map>

#include <MasterLog/LogExport.h>

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/Services/NetworkService/INetworkService.h>
#include <ucf/Services/ContactService/IContactService.h>
#include <ucf/Services/ImageService/IImageService.h>

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

#include "AppRunner/AppRunner.h"
#include "LoggerDefine.h"
namespace AppRunner
{
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class ApplicationRunner::DataPrivate
{
public:
    void createApp(const std::vector<std::string>& args);
    void initApp();
    void exitApp();
public:
    const FrameworkDependencies& getDependencies() const;
public:
    void createApplicationConfig();
    void parseCommandLines(const std::vector<std::string>& args);
    void createFrameworks();
    void initFrameworks();
    void exitFrameworks();
private:
    ApplicationConfig mApplicationConfig;
    std::map<std::string, std::string> mCommandLineValues;
    FrameworkDependencies mFrameworkDependencies;
};

void ApplicationRunner::DataPrivate::createApp(const std::vector<std::string>& args)
{
    //1. parse args and create application config
    //2. create frameworks
    parseCommandLines(args);
    createApplicationConfig();

    //init logger
	auto fileLogger = std::make_shared<LogLogSpace::LoggerFileConfigure>(
                        mApplicationConfig.appLogConfig.logLevel, 
                        mApplicationConfig.appLogConfig.logDirPath, 
                        mApplicationConfig.appLogConfig.logBaseFileName, 
                        mApplicationConfig.appLogConfig.logMaxKeepDays, 
                        mApplicationConfig.appLogConfig.logMaxSingleFileSize, 
                        mApplicationConfig.appLogConfig.loggerName
                    );
	MasterLogUtil::InitLogger({fileLogger});
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========Logger Initialzied==============");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
    
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========start create Frameworks=========");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
    createFrameworks();
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========Frameworks Created==============");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
}

void ApplicationRunner::DataPrivate::initApp()
{
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========start init App==================");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
    //1. init frameworks
    initFrameworks();
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========App initialized=================");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
}

void ApplicationRunner::DataPrivate::exitApp()
{
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========start exit App==================");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
    exitFrameworks();
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========App exited======================");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
}

void ApplicationRunner::DataPrivate::parseCommandLines(const std::vector<std::string>& args)
{
    if (args.empty())
    {
        return;
    }

    std::map<std::string, std::string> commandLineValues;
    for (const std::string& arg: args)
    {
        if (size_t pos = arg.find("="); pos != std::string::npos)
        {
            mCommandLineValues[arg.substr(0, pos)] = arg.substr(pos);
        }
    }
}

void ApplicationRunner::DataPrivate::createApplicationConfig()
{
    AppLogConfig logConfig{
        "app_logs",
        "AppLog",
        MasterLogUtil::ALL_LOG_LEVEL,
        180,
        50 * 1024 * 1024
    };
    
    mApplicationConfig.appLogConfig = logConfig;
}

void ApplicationRunner::DataPrivate::createFrameworks()
{
    mFrameworkDependencies.coreFramework = ucf::framework::ICoreFramework::CreateInstance();
    mFrameworkDependencies.commonHeadFramework = commonHead::ICommonHeadFramework::CreateInstance(mFrameworkDependencies.coreFramework);
}

void ApplicationRunner::DataPrivate::initFrameworks()
{
    if (mFrameworkDependencies.coreFramework)
    {
        mFrameworkDependencies.coreFramework->initCoreFramework();
        mFrameworkDependencies.coreFramework->registerService<ucf::service::IDataWarehouseService>(ucf::service::IDataWarehouseService::CreateInstance(mFrameworkDependencies.coreFramework));
        mFrameworkDependencies.coreFramework->registerService<ucf::service::INetworkService>(ucf::service::INetworkService::CreateInstance(mFrameworkDependencies.coreFramework));
        mFrameworkDependencies.coreFramework->registerService<ucf::service::IContactService>(ucf::service::IContactService::CreateInstance(mFrameworkDependencies.coreFramework));
        mFrameworkDependencies.coreFramework->registerService<ucf::service::IImageService>(ucf::service::IImageService::CreateInstance(mFrameworkDependencies.coreFramework));

        mFrameworkDependencies.coreFramework->initServices();
    }

    if (mFrameworkDependencies.commonHeadFramework)
    {
        mFrameworkDependencies.commonHeadFramework->initCommonheadFramework();
    }
}

void ApplicationRunner::DataPrivate::exitFrameworks()
{
    if (mFrameworkDependencies.coreFramework)
    {
        mFrameworkDependencies.coreFramework->exitCoreFramework();
    }

    if (mFrameworkDependencies.commonHeadFramework)
    {
        mFrameworkDependencies.commonHeadFramework->exitCommonheadFramework();
    }
}

const FrameworkDependencies& ApplicationRunner::DataPrivate::getDependencies() const
{
    return mFrameworkDependencies;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start AppRunner Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////


ApplicationRunner::ApplicationRunner()
    : mDataPrivate(std::make_unique<ApplicationRunner::DataPrivate>())
{

}

ApplicationRunner::~ApplicationRunner()
{

}

void ApplicationRunner::createApp(int argc, char *argv[])
{
    if (argc < 1)
    {
        return;
    }

    mDataPrivate->createApp(std::vector<std::string>(argv+1, argv + argc));
}

void ApplicationRunner::initApp()
{
    mDataPrivate->initApp();
}

void ApplicationRunner::exitApp()
{
    mDataPrivate->exitApp();
}

commonHead::ICommonHeadFrameworkWPtr ApplicationRunner::getCommonheadFramework() const
{
    return std::weak_ptr(mDataPrivate->getDependencies().commonHeadFramework);
}

ucf::framework::ICoreFrameworkWPtr ApplicationRunner::getCoreFramework() const
{
    return std::weak_ptr(mDataPrivate->getDependencies().coreFramework);
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish AppRunner Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
}
