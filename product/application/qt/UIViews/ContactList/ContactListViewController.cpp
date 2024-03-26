#include "ContactListViewController.h"


ContactListViewController::ContactListViewController(QObject *parent)
    : QObject(parent)
{

}

QString ContactListViewController::getControllerName()
{
    return "ContactListViewController";
}