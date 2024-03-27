#include "AppRunner/AppRunner.h"
#include "MainWindow/MainWindow.h"
#include "MasterLog/LogExport.h"

//for coreframework
static inline const char * MainLoggerName = "APP";
#define MAIN_LOG_DEBUG(message)     LOG_DEBUG("CoreFramework",message,MainLoggerName)
#define MAIN_LOG_INFO(message)      LOG_INFO("CoreFramework",message,MainLoggerName)
#define MAIN_LOG_WARN(message)      LOG_WARN("CoreFramework",message,MainLoggerName)
#define MAIN_LOG_ERROR(message)     LOG_ERROR("CoreFramework",message,MainLoggerName)
#define MAIN_LOG_FATAL(message)     LOG_FATAL("CoreFramework",message,MainLoggerName)

int main(int argc, char *argv[])
{
    AppRunner::AppLogConfig logConfig{
        "applogs",
        "temeplateTool",
        MasterLogUtil::ALL_LOG_LEVEL,
        180,
        50 * 1024 * 1024
    };
    AppRunner::ApplicationConfig appConfig{logConfig};
    auto dependencies = AppRunner::initAppDependencies(appConfig);
    MAIN_LOG_DEBUG("initAppDependencies done, start mainWindow");
    MainWindow a(dependencies.commonHeadFramework);
    return a.runMainWindow(argc, argv);
}