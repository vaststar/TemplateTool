#include <UTMenu/MenuItemModel.h>

MenuItemModel::MenuItemModel(const QString& name, const QString& action, QObject* parent)
    : QObject(parent)
    , m_name(name)
    , m_action(action)
{

}


QString MenuItemModel::name() const
{
    return m_name;
}

QString MenuItemModel::action() const
{
    return m_action;
}

QList<MenuItemModel*> MenuItemModel::subItems() const
{ 
    return m_subItems;
}

void MenuItemModel::addSubItem(MenuItemModel* item)
{
    m_subItems.append(item);
}