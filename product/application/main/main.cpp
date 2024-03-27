#include "AppRunner/AppRunner.h"
#include "MainWindow.h"
#include "MasterLog/LogExport.h"

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
    MainWindow a;
    return a.runMainWindow(argc, argv);
}