#include "ContactList/ContactListViewController.h"


ContactListViewController::ContactListViewController(QObject *parent)
    : BaseController(parent)
{

}

QString ContactListViewController::getControllerName() const
{
    return "ContactListViewController";
}
