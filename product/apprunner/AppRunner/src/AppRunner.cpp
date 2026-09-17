#include <algorithm>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

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
    void parseCommandLines(const std::vector<std::string>& args);
    void createFrameworks();
    void initFrameworks();
    void injectStartupParameters();
    void exitFrameworks();
    void initLogger();

private:
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
        parseCommandLines(args);
        initLogger();
        createFrameworks();
    });
}
void ApplicationRunner::DataPrivate::initLogger()
{
#if defined(_DEBUG) || !defined(NDEBUG)
    constexpr const char* APP_INTERNAL_NAME = "TemplateToolAppDebug";
#else
    constexpr const char* APP_INTERNAL_NAME = "TemplateToolApp";
#endif
    constexpr const char* APP_LOG_FOLDER_NAME = "app_log";
    constexpr const char* APP_LOG_BASE_NAME = "AppLog";

    const auto logDirPath = ucf::utilities::FilePathUtils::joinPaths(
        ucf::utilities::SystemUtils::getBaseStorageDir(),
        APP_INTERNAL_NAME,
        APP_LOG_FOLDER_NAME);
    ucf::utilities::FilePathUtils::EnsureDirectoryExists(logDirPath);

    ucf::utilities::FileOutputConfig fileOutput;
    fileOutput.minimumLevel = ucf::utilities::LogLevel::Debug;
    fileOutput.directory = logDirPath;
    fileOutput.baseName = APP_LOG_BASE_NAME;
    fileOutput.maxFileBytes = 50ULL * 1024ULL * 1024ULL;
    fileOutput.retentionDays = 180;
    fileOutput.calendarRotation = ucf::utilities::FileOutputConfig::CalendarRotation::Daily;

    ucf::utilities::LoggerConfig appLogger;
    appLogger.loggerName = ucf::utilities::kAppLoggerName;
    appLogger.fileOutputs.emplace_back(std::move(fileOutput));

#if defined(_DEBUG)
    ucf::utilities::ConsoleOutputConfig consoleOutput;
    consoleOutput.minimumLevel = ucf::utilities::LogLevel::Debug;
    consoleOutput.colorMode = ucf::utilities::ConsoleOutputConfig::ColorMode::Automatic;
    appLogger.consoleOutput = consoleOutput;
#endif

    ucf::utilities::LoggingConfig loggingConfig;
    loggingConfig.loggers.emplace_back(std::move(appLogger));
    ucf::utilities::initializeLogging(std::move(loggingConfig));

    mLoggerInitialized = true;
    RUNNER_LOG_INFO(
        "==================== Application run started ====================");
    RUNNER_LOG_INFO(
        "Logger initialized, directory: "
        << ucf::utilities::FilePathUtils::utf8FromPath(logDirPath)
        << ", baseFileName: "
        << APP_LOG_BASE_NAME);
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
        if (!arg.empty())
        {
            mCommandLineValues.push_back(arg);
        }
    });
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
        ucf::utilities::shutdownLogging();
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
