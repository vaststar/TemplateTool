#include "MainWindow/MainWindowController.h"

#include "CommonHeadFramework/ICommonHeadFramework.h"
#include "MainWindow/LoggerDefine.h"

#include "ClientGlobal/ClientGlobal.h"

#include "ContactListViewModel/IContactListViewModel.h"
#include "ContactListViewModel/ContactListModel.h"

MainWindowController::MainWindowController(QObject* parent)
    : BaseController(parent)
    , mCommonHeadFramework(ClientGlobal::getInstance()->getCommonHeadFramework())
{
    assert(mCommonHeadFramework.lock());
    
    auto contactListViewModel = CommonHead::ViewModels::IContactListViewModel::CreateInstance(mCommonHeadFramework);

    auto contactLists = contactListViewModel->getContactList();
    for(auto contact: contactLists)
    {
        MAINUI_LOG_DEBUG("found contact: " <<contact.getContactId());
    }
}

QString MainWindowController::getControllerName() const
{
    return "MainWindowController";
}

void MainWindowController::initController(ICommonHeadFrameworkWPtr commonheadFramework)
{
    
}