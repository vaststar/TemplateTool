#pragma once

#include <optional>
#include <vector>
#include <memory>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>

#include <commonHead/viewModels/ContactListViewModel/IContactListModel.h>

namespace commonHead{
    class ICommonHeadFramework;
    using ICommonHeadFrameworkWptr = std::weak_ptr<ICommonHeadFramework>;
}

namespace commonHead::viewModels{
class COMMONHEAD_EXPORT IContactListViewModelCallback
{
public:
    IContactListViewModelCallback() = default;
    IContactListViewModelCallback(const IContactListViewModelCallback&) = delete;
    IContactListViewModelCallback(IContactListViewModelCallback&&) = delete;
    IContactListViewModelCallback& operator=(const IContactListViewModelCallback&) = delete;
    IContactListViewModelCallback& operator=(IContactListViewModelCallback&&) = delete;
    virtual ~IContactListViewModelCallback() = default;
public:
    virtual void onContactDirectoryReady() {}
    virtual void onContactDirectoryLoadFailed(model::ContactDirectoryLoadError /*error*/) {}
    virtual void onPersonContactsAdded(const std::vector<model::ContactNodeData>& persons) {}
    virtual void onPersonContactsUpdated(const std::vector<model::ContactNodeData>& persons) {}
    virtual void onPersonContactsRemoved(const std::vector<std::string>& contactIds) {}
    virtual void onGroupContactsAdded(const std::vector<model::ContactNodeData>& groups) {}
    virtual void onGroupContactsUpdated(const std::vector<model::ContactNodeData>& groups) {}
    virtual void onGroupContactsRemoved(const std::vector<std::string>& contactIds) {}
    virtual void onContactRelationsAdded(const std::vector<model::ContactRelationData>& relations) {}
    virtual void onContactRelationsUpdated(const std::vector<model::ContactRelationData>& relations) {}
    // Carries (relationId, childId, oldParentId, type) for every relation row removed in
    // this slice. Subscribers should re-parent each childId locally (commonly to root in
    // single-parent trees) and may use oldParentId / type for diagnostics or to support
    // multi-parent / edge-precise UIs in the future.
    virtual void onContactRelationsRemoved(const std::vector<model::ContactRelationData>& removed) {}
};

class COMMONHEAD_EXPORT IContactListViewModel: public IViewModel, public virtual commonHead::utilities::IVMNotificationHelper<IContactListViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    IContactListViewModel(const IContactListViewModel&) = delete;
    IContactListViewModel(IContactListViewModel&&) = delete;
    IContactListViewModel& operator=(const IContactListViewModel&) = delete;
    IContactListViewModel& operator=(IContactListViewModel&&) = delete;
    virtual ~IContactListViewModel() = default;
public:
    virtual std::string getViewModelName() const = 0;
    virtual model::ContactTreePtr getContactList() const = 0;

    // Relation slice this VM is bound to (Department / Reporting / ...).
    [[nodiscard]] virtual model::RelationType getRelationType() const = 0;
    virtual void setRelationType(model::RelationType type) = 0;

    // True once the tree has been populated. onContactDirectoryReady will not be re-fired
    // for late subscribers.
    [[nodiscard]] virtual bool isContactDirectoryReady() const = 0;

    // Fire-and-forget metrics hook; VM does not store selection.
    virtual void selectContact(const std::string& contactId) = 0;

    [[nodiscard]] virtual std::optional<model::ContactDetail>
    getContactDetail(const std::string& contactId) const = 0;

    [[nodiscard]] virtual bool canMoveContact(const std::string& childId,
                                              const std::string& newParentId) const = 0;
    virtual void moveContact(const std::string& childId,
                             const std::string& newParentId) = 0;

    // ===== Create / edit / delete =====
    // Whether a node of the given type may be created directly under parentId. An empty
    // parentId means the virtual root. Mirrors canMoveContact: the rule lives in the VM
    // so the UI does not have to re-encode business constraints.
    [[nodiscard]] virtual bool canAddContact(const std::string& parentId,
                                             model::ContactNodeType type) const = 0;

    // Create a new contact (Person or Group) under parentId. The VM mints the contact id
    // and, when parentId is non-empty, attaches the node to the parent in the slice this
    // VM owns. Returns the freshly-minted contact id (empty on failure) so callers can
    // track the node once the add callbacks arrive.
    virtual std::string addContact(const std::string& parentId,
                                   const model::ContactNodeData& data) = 0;

    // Rename / re-state an existing contact. Only fields the slice understands are applied;
    // changing a group's groupType is not supported.
    virtual void updateContact(const model::ContactNodeData& data) = 0;

    [[nodiscard]] virtual bool canRemoveContact(const std::string& contactId) const = 0;

    // Remove the contact and detach the relation rows that reference it (its own parent
    // link plus the links to its children, which fall back to the virtual root).
    virtual void removeContact(const std::string& contactId) = 0;
public:
    static std::shared_ptr<IContactListViewModel> createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};
}
