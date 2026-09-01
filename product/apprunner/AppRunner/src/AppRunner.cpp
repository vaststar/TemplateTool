#include <optional>
#include <algorithm>
#include <mutex>

#include <ucf/utilities/FilePathUtils/FilePathUtils.h>
#include <ucf/utilities/SystemUtils/SystemUtils.h>

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/CoreFramework/CoreFrameworkCreator.h>
#include <ucf/services/ServiceFactory/IServiceFactory.h>

#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>

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
    [[nodiscard]] bool isLoggerInitialized() const noexcept;
private:
    void createApplicationConfig();
    void parseCommandLines(const std::vector<std::string>& args);
    void createFrameworks();
    void initFrameworks();
    void injectStartupParameters();
    void exitFrameworks();
    void initLogger();

private:
    ApplicationConfig mApplicationConfig;
    std::vector<std::string> mCommandLineValues;
    FrameworkDependencies mFrameworkDependencies;
    std::shared_ptr<ucf::service::IServiceFactory> mServiceFactory;
    std::once_flag mCreate_flag;
    std::once_flag mInit_flag;
    std::once_flag mExit_flag;
    bool mLoggerInitialized{false};
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
	auto fileLogger = std::make_shared<ucf::utilities::FileLoggerConfig>(
                        mApplicationConfig.appLogConfig.logLevel,
                        mApplicationConfig.appLogConfig.logDirPath,
                        mApplicationConfig.appLogConfig.logBaseFileName,
                        mApplicationConfig.appLogConfig.logMaxKeepDays,
                        mApplicationConfig.appLogConfig.logMaxSingleFileSize,
                        mApplicationConfig.appLogConfig.loggerName
                    );
#if defined(_DEBUG)
    auto consoleLogger = std::make_shared<ucf::utilities::ConsoleLoggerConfig>(mApplicationConfig.appLogConfig.logLevel, mApplicationConfig.appLogConfig.loggerName);
    std::vector<std::shared_ptr<ucf::utilities::LoggerConfig>> logConfigs = {fileLogger, consoleLogger};
    UCF_LOG_INIT(logConfigs);
#else
    std::vector<std::shared_ptr<ucf::utilities::LoggerConfig>> logConfigs = {fileLogger};
    UCF_LOG_INIT(logConfigs);
#endif
    mLoggerInitialized = true;
    RUNNER_LOG_INFO(
        "Logger initialized, directory: "
        << mApplicationConfig.appLogConfig.logDirPath
        << ", baseFileName: "
        << mApplicationConfig.appLogConfig.logBaseFileName);
}

bool ApplicationRunner::DataPrivate::isLoggerInitialized() const noexcept
{
    return mLoggerInitialized;
}

void ApplicationRunner::DataPrivate::initApp()
{
    std::call_once(mInit_flag, [this]() {
        RUNNER_LOG_INFO("Framework initialization started");

        initFrameworks();
        injectStartupParameters();

        RUNNER_LOG_INFO("Framework initialization finished");
    });
}

void ApplicationRunner::DataPrivate::exitApp()
{
    std::call_once(mExit_flag, [this]() {
        RUNNER_LOG_INFO("Framework shutdown started");

        exitFrameworks();

        RUNNER_LOG_INFO("Framework shutdown finished");
    });
}

void ApplicationRunner::DataPrivate::parseCommandLines(const std::vector<std::string>& args)
{
    if (args.empty())
    {
        return;
    }

    std::for_each(args.cbegin(), args.cend(), [this](const std::string& arg){
        if(!arg.empty())
        {
            mCommandLineValues.push_back(arg);
        }
    });
}

void ApplicationRunner::DataPrivate::createApplicationConfig()
{
    //1, setup log config
    constexpr const char* APP_INTERNAL_NAME = "TemplateToolApp";
    constexpr const char* APP_INTERNAL_NAME_DEBUG = "TemplateToolAppDebug";
    constexpr const char* APP_LOG_FOLDER_NAME = "app_log";
    std::filesystem::path logDirPath;
#if defined(_DEBUG) || !defined(NDEBUG)
    logDirPath = ucf::utilities::FilePathUtils::joinPaths(
        ucf::utilities::SystemUtils::getBaseStorageDir(),
        APP_INTERNAL_NAME_DEBUG,
        APP_LOG_FOLDER_NAME
    );
#else
    logDirPath = ucf::utilities::FilePathUtils::joinPaths(
        ucf::utilities::SystemUtils::getBaseStorageDir(),
        APP_INTERNAL_NAME,
        APP_LOG_FOLDER_NAME
    );
#endif
    ucf::utilities::FilePathUtils::EnsureDirectoryExists(logDirPath);

    AppLogConfig logConfig{
        logDirPath.string(),
        "AppLog",
        ucf::utilities::kAllLogLevels,
        180,
        50 * 1024 * 1024
    };

    mApplicationConfig.appLogConfig = logConfig;
}

void ApplicationRunner::DataPrivate::createFrameworks()
{
    RUNNER_LOG_INFO("Framework creation started");

    mFrameworkDependencies.coreFramework = ucf::framework::createCoreFramework();
    mServiceFactory = ucf::service::IServiceFactory::createInstance(mFrameworkDependencies.coreFramework);
    mFrameworkDependencies.commonHeadFramework = commonHead::ICommonHeadFramework::createInstance(mFrameworkDependencies.coreFramework);

    RUNNER_LOG_INFO("Framework creation finished");
}

void ApplicationRunner::DataPrivate::initFrameworks()
{
    if (mFrameworkDependencies.coreFramework)
    {
        mFrameworkDependencies.coreFramework->initCoreFramework();
        mServiceFactory->createServices();

        mFrameworkDependencies.coreFramework->initServices();
    }

    if (mFrameworkDependencies.commonHeadFramework)
    {
        mFrameworkDependencies.commonHeadFramework->initCommonheadFramework();
    }
}

void ApplicationRunner::DataPrivate::injectStartupParameters()
{
    if (!mCommandLineValues.empty())
    {
        RUNNER_LOG_DEBUG("Will set command line args, size: " << mCommandLineValues.size());
        if (auto coreFramework = mFrameworkDependencies.coreFramework)
        {
            coreFramework->setStartupParameters(mCommandLineValues);
        }
    }
}

void ApplicationRunner::DataPrivate::exitFrameworks()
{
    if (mFrameworkDependencies.commonHeadFramework)
    {
        mFrameworkDependencies.commonHeadFramework->exitCommonheadFramework();
    }

    if (mFrameworkDependencies.coreFramework)
    {
        mFrameworkDependencies.coreFramework->exitCoreFramework();
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
    const bool loggerInitialized =
        mDataPrivate && mDataPrivate->isLoggerInitialized();

    if (loggerInitialized)
    {
        RUNNER_LOG_INFO(
            "ApplicationRunner is releasing owned dependencies before logger shutdown");
    }

    // Destroy frameworks and services while the logger is still available so
    // their destructors can finish writing diagnostic information.
    mDataPrivate.reset();

    if (loggerInitialized)
    {
        RUNNER_LOG_INFO(
            "ApplicationRunner owned dependencies released, stopping logger");
        RUNNER_LOG_INFO(
            "==================== Application run ended ====================");
        UCF_LOG_STOP();
    }
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
