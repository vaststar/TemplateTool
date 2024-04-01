#include "MainWindow/MainWindowController.h"
#include "CommonHeadFramework/ICommonHeadFramework.h"
#include "MainWindow/LoggerDefine.h"

#include "ContactListViewModel/IContactListViewModel.h"
#include "ContactListViewModel/ContactListModel.h"

MainWindowController::MainWindowController(ICommonHeadFrameworkWPtr commonHeadFramework)
:mCommonHeadFramework(commonHeadFramework)
{

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