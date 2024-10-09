#include "Main/Main.h"


#include <ucf/CoreFramework/ICoreFramework.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

#include "AppRunner/AppRunner.h"
#include "Main/LoggerDefine.h"

#include "AppUIManager/include/AppUIManager.h"



/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class Main::DataPrivate
{
public:
    void initDataPrivate();
    void exitApp();
    const AppRunner::FrameworkDependencies& getDependencies() const{ return mDependencies;}
private:
    AppRunner::FrameworkDependencies mDependencies;
};

void Main::DataPrivate::initDataPrivate()
{
    AppRunner::AppLogConfig logConfig{
        "applogs",
        "temeplateTool",
        MasterLogUtil::ALL_LOG_LEVEL,
        180,
        50 * 1024 * 1024
    };
    
    AppRunner::ApplicationConfig appConfig{logConfig};
    mDependencies = AppRunner::initAppDependencies(appConfig);
}

void Main::DataPrivate::exitApp()
{
    if (mDependencies.coreFramework)
    {
        mDependencies.coreFramework->exitCoreFramework();
    }

    if (mDependencies.commonHeadFramework)
    {
        mDependencies.commonHeadFramework->exitCommonheadFramework();
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
Main::Main()
    : mDataPrivate(std::make_unique<Main::DataPrivate>())
{
}

Main::~Main()
{
    MAINUI_LOG_DEBUG("delete Main");
    LOG_WAIT_EXIT();
}

int Main::runMain(int argc, char *argv[])
{
    mDataPrivate->initDataPrivate();
    MAINUI_LOG_DEBUG("init dependencies done");

    MAINUI_LOG_DEBUG(__cplusplus);
    int runResult = 0;
    {
        AppUIManager AppUIManager(AppUIManager::ApplicationConfig{argc, argv, std::weak_ptr(mDataPrivate->getDependencies().commonHeadFramework)});
        runResult = AppUIManager.runApp();
    }
    
    MAINUI_LOG_DEBUG("start quit App");
    mDataPrivate->exitApp();
    MAINUI_LOG_DEBUG("finish quit App");
    return runResult;
}