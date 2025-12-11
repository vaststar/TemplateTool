#include <vector>
#include <string>

#include "Main/Main.h"


#include <ucf/CoreFramework/ICoreFramework.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

#include "AppRunner/AppRunner.h"
#include "LoggerDefine.h"

#include "AppUIManager/include/AppUIManager.h"



/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
class Main::DataPrivate
{
public:
    void initApp(int argc, char *argv[]);
    void exitApp();
    commonHead::ICommonHeadFrameworkWPtr getCommonHeadFramework(){ return mApplicationRunner.getCommonheadFramework();}
private:
    AppRunner::ApplicationRunner mApplicationRunner;
};

void Main::DataPrivate::initApp(int argc, char *argv[])
{
    mApplicationRunner.initApp(argc, argv);
}

void Main::DataPrivate::exitApp()
{
    mApplicationRunner.exitApp();
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
    mDataPrivate->initApp(argc, argv);
    MAINUI_LOG_DEBUG("init dependencies done");

    MAINUI_LOG_DEBUG("===========================================");
    MAINUI_LOG_DEBUG("===========================================");
    MAINUI_LOG_DEBUG("===========start run mainApp===============");
    MAINUI_LOG_DEBUG("===========================================");
    MAINUI_LOG_DEBUG("===========================================");
    int runResult = 0;
    {
        AppUIManager AppUIManager(AppUIManager::ApplicationConfig{argc, argv, mDataPrivate->getCommonHeadFramework()});
        runResult = AppUIManager.runApp();
    }
    MAINUI_LOG_DEBUG("===========mainApp run result:" << runResult << "=======");
    
    MAINUI_LOG_DEBUG("===========================================");
    MAINUI_LOG_DEBUG("===========start quit mainApp==============");
    MAINUI_LOG_DEBUG("===========================================");
    mDataPrivate->exitApp();
    MAINUI_LOG_DEBUG("===========================================");
    MAINUI_LOG_DEBUG("===========finish quit mainApp=============");
    MAINUI_LOG_DEBUG("===========================================");
    return runResult;
}