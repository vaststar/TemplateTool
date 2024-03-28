#include "MainWindow/MainWindowController.h"
#include "CommonHeadFramework/ICommonHeadFramework.h"

MainWindowController::MainWindowController(ICommonHeadFrameworkWPtr commonHeadFramework)
:mCommonHeadFramework(commonHeadFramework)
{

}

QString MainWindowController::getControllerName() const
{
    return "MainWindowController";
}