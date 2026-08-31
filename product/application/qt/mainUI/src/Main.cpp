#include <vector>
#include <string>

#include "Main/Main.h"


#include <ucf/CoreFramework/ICoreFramework.h>
#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>

#include <AppRunner/AppRunner.h>
#include <UISingleInstance/UISingleInstanceChecker.h>
#include <UIIPCChannel/UIIPCClient.h>
#include "LoggerDefine.h"

#include <AppUIManager/AppUIManager.h>

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
static constexpr auto SINGLE_INSTANCE_NAME = "TemplateTool_SingleInstance";
static constexpr auto IPC_SERVER_NAME = "TemplateTool_IPC_Server";

class Main::DataPrivate
{
public:
    int runApp(int argc, char *argv[]);
    void exitApp();
    commonHead::ICommonHeadFrameworkWPtr getCommonHeadFramework(){ return mApplicationRunner.getCommonheadFramework();}
private:
    void initApp(int argc, char *argv[]);
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

int Main::DataPrivate::runApp(int argc, char *argv[])
{
    initApp(argc, argv);
    MAINUI_LOG_DEBUG("Application dependencies initialized");

    MAINUI_LOG_DEBUG("UI application execution started");

    int runResult = 0;
    {
        AppUIManager appUIManager(
            AppUIManager::ApplicationConfig{
                argc,
                argv,
                getCommonHeadFramework()});
        runResult = appUIManager.runApp();
    }

    MAINUI_LOG_DEBUG(
        "UI application execution finished, exitCode: "
        << runResult);

    exitApp();

    return runResult;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Finish DataPrivate Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
Main::Main()
    : mDataPrivate(std::make_unique<Main::DataPrivate>())
{
    MAINUI_LOG_DEBUG("Main constructed, address: " << this);
}

Main::~Main()
{
    MAINUI_LOG_DEBUG("Main destroying, address: " << this);
}

int Main::runMain(int argc, char *argv[])
{
    if (UIUtilities::UISingleInstanceChecker singleInstanceChecker(SINGLE_INSTANCE_NAME); singleInstanceChecker.tryToRun())
    {
        int result = mDataPrivate->runApp(argc, argv);
        return result;
    }
    else
    {
        UIUtilities::UIIPCClient client(IPC_SERVER_NAME);
        client.send("ActivateWindow");
    }
    return -1;
}
