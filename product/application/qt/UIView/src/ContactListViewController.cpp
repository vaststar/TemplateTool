#include "ContactList/ContactListViewController.h"
#include "LoggerDefine.h"
#include <commonHead/viewModels/ContactListViewModel/IContactListViewModel.h>
#include <commonHead/viewModels/ContactListViewModel/ContactListModel.h>


ContactListViewController::ContactListViewController(QObject *parent)
    : BaseController(parent)
{
    mContactListViewModel = commonHead::viewModels::IContactListViewModel::CreateInstance(getCommonHeadFramework());
    assert(mContactListViewModel);
    UIVIEW_LOG_DEBUG("create ContactListViewController");
}

QString ContactListViewController::getControllerName() const
{
    return "ContactListViewController";
}

void ContactListViewController::buttonClicked()
{
    mContactListViewModel->getContactList();
}