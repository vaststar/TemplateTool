#include "ContactList/ContactListViewController.h"
#include "LoggerDefine.h"
#include <ContactListViewModel/IContactListViewModel.h>
#include <ContactListViewModel/ContactListModel.h>


ContactListViewController::ContactListViewController(QObject *parent)
    : BaseController(parent)
{
    mContactListViewModel = CommonHead::ViewModels::IContactListViewModel::CreateInstance(getCommonHeadFramework());
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