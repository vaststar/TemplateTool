#include "ContactList/ContactListViewController.h"
#include "LoggerDefine.h"


ContactListViewController::ContactListViewController(QObject *parent)
    : BaseController(parent)
{
    UIVIEW_LOG_DEBUG("create ContactListViewController");
}

QString ContactListViewController::getControllerName() const
{
    return "ContactListViewController";
}
