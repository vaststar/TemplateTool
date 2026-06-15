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
    // The detail of the currently-selected contact. Recomputed and re-notified whenever the
    // selection changes or that contact's data is updated in the VM, so the detail panel can
    // simply bind to it instead of imperatively re-fetching.
    Q_PROPERTY(QVariantMap currentContactInfo READ getCurrentContactInfo NOTIFY currentContactInfoChanged)
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
    QVariantMap getCurrentContactInfo() const;
    LoadState getLoadState() const;

public slots:
    void buttonClicked();
    Q_INVOKABLE void selectContact(const QString& contactId);
    Q_INVOKABLE QVariantMap getContactInfo(const QString& contactId) const;
    Q_INVOKABLE bool canDropOn(const QString& srcId, const QString& targetParentId) const;
    Q_INVOKABLE void moveContact(const QString& srcId, const QString& targetParentId);

    // Right-click context menu. The panel asks for the menu model (built from the
    // real add/remove permissions) and then reports back the chosen action; the
    // controller decides which dialog to open. nodeType: -1 = blank/root, 0 = Person, 1 = Group.
    Q_INVOKABLE QVariantList contextMenuModel(const QString& contactId, int nodeType) const;
    Q_INVOKABLE void handleContextAction(const QString& action, const QString& contactId, int nodeType);

    // Dialog commands, invoked by the edit / delete dialog windows. fields carries
    // {displayName, nodeType, groupType} for add and {displayName} for edit.
    Q_INVOKABLE void addContact(const QString& parentId, const QVariantMap& fields);
    Q_INVOKABLE void updateContact(const QString& contactId, const QVariantMap& fields);
    Q_INVOKABLE void removeContact(const QString& contactId);

protected:
    void init() override;

private slots:
    void onContactDirectoryReady();
    void onContactDirectoryLoadFailed(commonHead::viewModels::model::ContactDirectoryLoadError error);
    void onPersonContactsAdded   (const std::vector<commonHead::viewModels::model::ContactNodeData>& v);
    void onPersonContactsUpdated (const std::vector<commonHead::viewModels::model::ContactNodeData>& v);
    void onPersonContactsRemoved (const std::vector<std::string>& v);
    void onGroupContactsAdded    (const std::vector<commonHead::viewModels::model::ContactNodeData>& v);
    void onGroupContactsUpdated  (const std::vector<commonHead::viewModels::model::ContactNodeData>& v);
    void onGroupContactsRemoved  (const std::vector<std::string>& v);
    void onContactRelationsAdded  (const std::vector<commonHead::viewModels::model::ContactRelationData>& v);
    void onContactRelationsUpdated(const std::vector<commonHead::viewModels::model::ContactRelationData>& v);
    void onContactRelationsRemoved(const std::vector<commonHead::viewModels::model::ContactRelationData>& v);

signals:
    void orgTreeModelChanged();
    void currentContactIdChanged();
    void currentContactInfoChanged();
    void loadStateChanged();
    void nodeMoved(QString newParentId);
    // Fired once a freshly-added contact has materialised in the tree model, so the view
    // can expand its parent and select it.
    void contactAdded(QString newId, QString parentId);

private:
    void setLoadState(LoadState s);
    bool canAddUnder(const QString& parentId, int nodeType) const;
    bool canRemove(const QString& contactId) const;
    // Re-notify currentContactInfo if the currently-selected contact is among the updated nodes.
    void notifyInfoIfCurrentUpdated(const std::vector<commonHead::viewModels::model::ContactNodeData>& v);
    // Spawn the standalone dialog windows (created via the view factory, centered on the
    // app window) and inject this controller plus the initial field values.
    void openEditDialog(const QString& mode, const QString& parentId, const QString& editId,
                        int nodeType, const QString& initialName);
    void openDeleteDialog(const QString& contactId);

private:
    std::shared_ptr<commonHead::viewModels::IContactListViewModel> mContactListViewModel;
    std::shared_ptr<UIVMSignalEmitter::ContactListViewModelEmitter> mContactListEmitter;
    ContactListItemModel* mOrgTreeModel = nullptr;
    QString m_currentContactId;
    LoadState m_loadState = Loading;
    bool    mHasPendingMove = false;
    QString mPendingMoveParent;
    QString mPendingAddId;
    QString mPendingAddParent;
};
