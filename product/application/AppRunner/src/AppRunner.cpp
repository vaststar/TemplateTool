#include <optional>
#include <map>
#include <mutex>

#include <MasterLog/LogExport.h>

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ClientInfoService/IClientInfoService.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseService.h>
#include <ucf/Services/NetworkService/INetworkService.h>
#include <ucf/Services/ContactService/IContactService.h>
#include <ucf/Services/ImageService/IImageService.h>
#include <ucf/Services/MediaService/IMediaService.h>

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
    const FrameworkDependencies& getDependencies() const;
private:
    void createApplicationConfig();
    void parseCommandLines(const std::vector<std::string>& args);
    void createFrameworks();
    void initFrameworks();
    void exitFrameworks();
    void initLogger();
private:
    ApplicationConfig mApplicationConfig;
    std::map<std::string, std::string> mCommandLineValues;
    FrameworkDependencies mFrameworkDependencies;
    std::once_flag mCreate_flag;
    std::once_flag mInit_flag;
    std::once_flag mExit_flag;
};

void ApplicationRunner::DataPrivate::createApp(const std::vector<std::string>& args)
{
    std::call_once(mCreate_flag, [args, this](){
        //1. parse args and create application config
        //2. create frameworks
        parseCommandLines(args);
        createApplicationConfig();

        //init logger
        initLogger();

        createFrameworks();
    });
}
void ApplicationRunner::DataPrivate::initLogger()
{
	auto fileLogger = std::make_shared<LogLogSpace::LoggerFileConfigure>(
                        mApplicationConfig.appLogConfig.logLevel, 
                        mApplicationConfig.appLogConfig.logDirPath, 
                        mApplicationConfig.appLogConfig.logBaseFileName, 
                        mApplicationConfig.appLogConfig.logMaxKeepDays, 
                        mApplicationConfig.appLogConfig.logMaxSingleFileSize, 
                        mApplicationConfig.appLogConfig.loggerName
                    );
#if defined(_DEBUG)
    auto consoleLogger = std::make_shared<LogLogSpace::LoggerConsoleConfigure>(mApplicationConfig.appLogConfig.logLevel, mApplicationConfig.appLogConfig.loggerName);
	MasterLogUtil::InitLogger({fileLogger, consoleLogger});
#else
	MasterLogUtil::InitLogger({fileLogger});
#endif
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========Logger Initialzied==============");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
}

void ApplicationRunner::DataPrivate::initApp()
{
    std::call_once(mInit_flag, [this](){
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
    });
}

void ApplicationRunner::DataPrivate::exitApp()
{
    std::call_once(mExit_flag, [this](){
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
    });
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
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========start create Frameworks=========");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
    mFrameworkDependencies.coreFramework = ucf::framework::ICoreFramework::createInstance();
    mFrameworkDependencies.commonHeadFramework = commonHead::ICommonHeadFramework::createInstance(mFrameworkDependencies.coreFramework);
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========Frameworks Created==============");
    RUNNER_LOG_INFO("===========================================");
    RUNNER_LOG_INFO("===========================================");
}

void ApplicationRunner::DataPrivate::initFrameworks()
{
    if (mFrameworkDependencies.coreFramework)
    {
        mFrameworkDependencies.coreFramework->initCoreFramework();
        mFrameworkDependencies.coreFramework->registerService<ucf::service::IDataWarehouseService>(ucf::service::IDataWarehouseService::createInstance(mFrameworkDependencies.coreFramework));
        mFrameworkDependencies.coreFramework->registerService<ucf::service::IClientInfoService>(ucf::service::IClientInfoService::createInstance(mFrameworkDependencies.coreFramework));
        mFrameworkDependencies.coreFramework->registerService<ucf::service::INetworkService>(ucf::service::INetworkService::createInstance(mFrameworkDependencies.coreFramework));
        mFrameworkDependencies.coreFramework->registerService<ucf::service::IContactService>(ucf::service::IContactService::createInstance(mFrameworkDependencies.coreFramework));
        mFrameworkDependencies.coreFramework->registerService<ucf::service::IImageService>(ucf::service::IImageService::createInstance(mFrameworkDependencies.coreFramework));
        mFrameworkDependencies.coreFramework->registerService<ucf::service::IMediaService>(ucf::service::IMediaService::createInstance(mFrameworkDependencies.coreFramework));
        
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

void ApplicationRunner::initApp(int argc, char *argv[])
{
    if (argc < 1)
    {
        return;
    }

    mDataPrivate->createApp(std::vector<std::string>(argv+1, argv + argc));
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
