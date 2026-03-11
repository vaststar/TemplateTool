#pragma once

#include <QObject>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"
#include "ContactListItemModel.h"

namespace commonHead::viewModels
{
    class IContactListViewModel;
}

class ContactsPageController : public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* orgTreeModel READ getOrgTreeModel NOTIFY orgTreeModelChanged)
    Q_PROPERTY(QString currentContactId READ getCurrentContactId NOTIFY currentContactIdChanged)
    QML_ELEMENT

public:
    explicit ContactsPageController(QObject* parent = nullptr);
    QAbstractItemModel* getOrgTreeModel() const;
    QString getCurrentContactId() const;

public slots:
    void buttonClicked();
    Q_INVOKABLE void selectContact(const QString& contactId);
    Q_INVOKABLE QVariantMap getContactInfo(const QString& contactId) const;

protected:
    void init() override;

private:
    void buildContactTreeModel();

signals:
    void orgTreeModelChanged();
    void currentContactIdChanged();

private:
    std::shared_ptr<commonHead::viewModels::IContactListViewModel> mContactListViewModel;
    ContactListItemModel* mOrgTreeModel = nullptr;
    QString m_currentContactId;
};
