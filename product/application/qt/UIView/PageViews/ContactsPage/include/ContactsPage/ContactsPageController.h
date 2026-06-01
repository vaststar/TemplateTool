#pragma once

#include <QObject>
#include <QtQml>
#include <memory>
#include <string>
#include <vector>

#include "UIViewBase/UIViewController.h"
#include "ContactListItemModel.h"
// Pull in Q_DECLARE_METATYPE specializations BEFORE moc generates code for
// our slot signatures.
#include "ViewModelSingalEmitter/RegisterViewModelMetaTypes.h"
#include <commonHead/viewModels/ContactListViewModel/IContactListModel.h>

namespace commonHead::viewModels
{
    class IContactListViewModel;
}
namespace UIVMSignalEmitter
{
    class ContactListViewModelEmitter;
}

class ContactsPageController : public UIViewController
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* orgTreeModel READ getOrgTreeModel NOTIFY orgTreeModelChanged)
    Q_PROPERTY(QString currentContactId READ getCurrentContactId NOTIFY currentContactIdChanged)
    Q_PROPERTY(LoadState loadState READ getLoadState NOTIFY loadStateChanged)
    QML_ELEMENT

public:
    enum LoadState
    {
        Loading = 0,
        Ready   = 1,
        Error   = 2,
    };
    Q_ENUM(LoadState)

    explicit ContactsPageController(QObject* parent = nullptr);
    ~ContactsPageController() override;

    QAbstractItemModel* getOrgTreeModel() const;
    QString getCurrentContactId() const;
    LoadState getLoadState() const;

public slots:
    void buttonClicked();
    Q_INVOKABLE void selectContact(const QString& contactId);
    Q_INVOKABLE QVariantMap getContactInfo(const QString& contactId) const;

protected:
    void init() override;

private slots:
    void onContactDirectoryReady();
    void onPersonContactsAdded   (const std::vector<commonHead::viewModels::model::ContactNodeData>& v);
    void onPersonContactsUpdated (const std::vector<commonHead::viewModels::model::ContactNodeData>& v);
    void onPersonContactsRemoved (const std::vector<std::string>& v);
    void onGroupContactsAdded    (const std::vector<commonHead::viewModels::model::ContactNodeData>& v);
    void onGroupContactsUpdated  (const std::vector<commonHead::viewModels::model::ContactNodeData>& v);
    void onGroupContactsRemoved  (const std::vector<std::string>& v);
    void onContactRelationsAdded  (const std::vector<commonHead::viewModels::model::ContactRelationData>& v);
    void onContactRelationsUpdated(const std::vector<commonHead::viewModels::model::ContactRelationData>& v);
    void onContactRelationsRemoved(const std::vector<std::string>& v);

signals:
    void orgTreeModelChanged();
    void currentContactIdChanged();
    void loadStateChanged();

private:
    void setLoadState(LoadState s);

private:
    std::shared_ptr<commonHead::viewModels::IContactListViewModel> mContactListViewModel;
    std::shared_ptr<UIVMSignalEmitter::ContactListViewModelEmitter> mContactListEmitter;
    ContactListItemModel* mOrgTreeModel = nullptr;
    QString m_currentContactId;
    LoadState m_loadState = Loading;
};
