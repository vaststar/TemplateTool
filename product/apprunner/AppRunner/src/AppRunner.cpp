#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <ucf/utilities/FilePathUtils/FilePathUtils.h>
#include <ucf/utilities/SystemUtils/SystemUtils.h>

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/CoreFramework/CoreFrameworkCreator.h>
#include <ucf/services/InvocationService/IInvocationService.h>
#include <ucf/services/InvocationService/StartupContext.h>
#include <ucf/services/ServiceFactory/IServiceFactory.h>

#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>

#include "AppRunner/AppRunner.h"
#include "LoggerDefine.h"
namespace AppRunner
{
namespace {

class StartupArgumentParser final
{
public:
    [[nodiscard]] ApplicationConfig parse(const std::vector<std::string>& arguments) const
    {
        ApplicationConfig applicationConfig;
        applicationConfig.startupConfig.commandLineArguments = arguments;
        return applicationConfig;
    }
};

[[nodiscard]] ucf::utilities::LogLevel toLoggerLevel(AppLogConfig::Level level)
{
    switch (level)
    {
    case AppLogConfig::Level::Debug:
        return ucf::utilities::LogLevel::Debug;
    case AppLogConfig::Level::Info:
        return ucf::utilities::LogLevel::Info;
    case AppLogConfig::Level::Warn:
        return ucf::utilities::LogLevel::Warn;
    case AppLogConfig::Level::Error:
        return ucf::utilities::LogLevel::Error;
    case AppLogConfig::Level::Fatal:
        return ucf::utilities::LogLevel::Fatal;
    case AppLogConfig::Level::Off:
        return ucf::utilities::LogLevel::Off;
    }

    throw std::invalid_argument{"AppRunner log level is invalid"};
}

[[nodiscard]] ucf::utilities::FileOutputConfig::CalendarRotation toLoggerRotation(AppLogConfig::CalendarRotation rotation)
{
    switch (rotation)
    {
    case AppLogConfig::CalendarRotation::None:
        return ucf::utilities::FileOutputConfig::CalendarRotation::None;
    case AppLogConfig::CalendarRotation::Daily:
        return ucf::utilities::FileOutputConfig::CalendarRotation::Daily;
    case AppLogConfig::CalendarRotation::Monthly:
        return ucf::utilities::FileOutputConfig::CalendarRotation::Monthly;
    }

    throw std::invalid_argument{"AppRunner log calendar rotation is invalid"};
}

} // namespace

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
    void createApplicationConfig(const std::vector<std::string>& args);
    void createFrameworks();
    void initFrameworks();
    void processStartupParameters();
    void exitFrameworks();
    void initLogger();

private:
    ApplicationConfig mApplicationConfig;
    FrameworkDependencies mFrameworkDependencies;
    std::shared_ptr<ucf::service::IServiceFactory> mServiceFactory;
    std::once_flag mCreate_flag;
    std::once_flag mInit_flag;
    std::once_flag mExit_flag;
    bool mLoggerInitialized{false};
};

void ApplicationRunner::DataPrivate::createApp(const std::vector<std::string>& args)
{
    std::call_once(mCreate_flag, [args, this]() {
        createApplicationConfig(args);
        initLogger();
        createFrameworks();
    });
}

void ApplicationRunner::DataPrivate::createApplicationConfig(const std::vector<std::string>& args)
{
    mApplicationConfig = StartupArgumentParser{}.parse(args);
}

void ApplicationRunner::DataPrivate::initLogger()
{
    auto& logConfig = mApplicationConfig.appLogConfig;

    if (logConfig.directory.empty())
    {
#if defined(_DEBUG) || !defined(NDEBUG)
        constexpr const char* APP_INTERNAL_NAME = "TemplateToolAppDebug";
#else
        constexpr const char* APP_INTERNAL_NAME = "TemplateToolApp";
#endif
        constexpr const char* APP_LOG_FOLDER_NAME = "app_log";

        logConfig.directory = ucf::utilities::FilePathUtils::joinPaths(
            ucf::utilities::SystemUtils::getBaseStorageDir(), APP_INTERNAL_NAME, APP_LOG_FOLDER_NAME);
    }
    ucf::utilities::FilePathUtils::EnsureDirectoryExists(logConfig.directory);

    ucf::utilities::FileOutputConfig fileOutput;
    fileOutput.minimumLevel = toLoggerLevel(logConfig.minimumLevel);
    fileOutput.directory = logConfig.directory;
    fileOutput.baseName = logConfig.baseName;
    fileOutput.maxFileBytes = logConfig.maxFileBytes;
    fileOutput.retentionDays = logConfig.retentionDays;
    fileOutput.calendarRotation = toLoggerRotation(logConfig.calendarRotation);

    ucf::utilities::LoggerConfig appLogger;
    appLogger.loggerName = ucf::utilities::kAppLoggerName;
    appLogger.fileOutputs.emplace_back(std::move(fileOutput));

#if defined(_DEBUG) || !defined(NDEBUG)
    ucf::utilities::ConsoleOutputConfig consoleOutput;
    consoleOutput.minimumLevel = toLoggerLevel(logConfig.minimumLevel);
    consoleOutput.colorMode = ucf::utilities::ConsoleOutputConfig::ColorMode::Automatic;
    appLogger.consoleOutput = consoleOutput;
#endif

    ucf::utilities::LoggingConfig loggingConfig;
    loggingConfig.loggers.emplace_back(std::move(appLogger));
    ucf::utilities::initializeLogging(std::move(loggingConfig));

    mLoggerInitialized = true;
    RUNNER_LOG_INFO("==================== Application run started ====================");
    RUNNER_LOG_INFO("Logger initialized, directory: "
        << ucf::utilities::FilePathUtils::utf8FromPath(logConfig.directory)
        << ", baseFileName: " << ucf::utilities::FilePathUtils::utf8FromPath(logConfig.baseName));
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
        processStartupParameters();

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

void ApplicationRunner::DataPrivate::processStartupParameters()
{
    auto coreFramework = mFrameworkDependencies.coreFramework;
    if (!coreFramework)
    {
        RUNNER_LOG_ERROR("Cannot process startup parameters: CoreFramework is unavailable");
        return;
    }

    auto invocationService = coreFramework->getService<ucf::service::IInvocationService>().lock();
    if (!invocationService)
    {
        RUNNER_LOG_ERROR("Cannot process startup parameters: InvocationService is unavailable");
        return;
    }

    ucf::service::StartupContext startupContext;
    startupContext.commandLineArguments = mApplicationConfig.startupConfig.commandLineArguments;

    RUNNER_LOG_DEBUG("Will process startup parameters, size: " << startupContext.commandLineArguments.size());
    invocationService->processStartupParameters(std::move(startupContext));
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
    const bool loggerInitialized = mDataPrivate && mDataPrivate->isLoggerInitialized();

    if (loggerInitialized)
    {
        RUNNER_LOG_INFO("ApplicationRunner is releasing owned dependencies before logger shutdown");
    }

    // Destroy frameworks and services while the logger is still available so
    // their destructors can finish writing diagnostic information.
    mDataPrivate.reset();

    if (loggerInitialized)
    {
        RUNNER_LOG_INFO("ApplicationRunner owned dependencies released, stopping logger");
        RUNNER_LOG_INFO("==================== Application run ended ====================");
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
