#include "ContactListViewController.h"


ContactListViewController::ContactListViewController(QObject *parent)
    : BaseController(parent)
{

}

QString ContactListViewController::getControllerName()
{
    return "ContactListViewController";
}

void ContactListViewController::initController(ICommonHeadFrameworkWPtr commonheadFramework)
{
    setCommonHeadFramework(commonheadFramework);
}
