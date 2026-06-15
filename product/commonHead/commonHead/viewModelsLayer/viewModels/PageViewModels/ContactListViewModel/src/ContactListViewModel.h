#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/ContactListViewModel/IContactListViewModel.h>
#include <ucf/Services/ContactService/IContactServiceCallback.h>

namespace ucf::service {
    class IContactService;
}

namespace commonHead::viewModels::model {
    class ContactTree;
}

namespace commonHead::viewModels{

class ContactListViewModel: public virtual IContactListViewModel,
                            public virtual commonHead::utilities::VMNotificationHelper<IContactListViewModelCallback>,
                            public ucf::service::IContactServiceCallback,
                            public std::enable_shared_from_this<ContactListViewModel>
{
public:
    explicit ContactListViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    ContactListViewModel() = default;
    ContactListViewModel(const ContactListViewModel&) = delete;
    ContactListViewModel(ContactListViewModel&&) = delete;
    ContactListViewModel& operator=(const ContactListViewModel&) = delete;
    ContactListViewModel& operator=(ContactListViewModel&&) = delete;
    ~ContactListViewModel() = default;
public:
    virtual std::string getViewModelName() const override;
    virtual model::ContactTreePtr getContactList() const override;
    model::RelationType getRelationType() const override;
    void setRelationType(model::RelationType type) override;
    bool isContactDirectoryReady() const override;
    void selectContact(const std::string& contactId) override;
    std::optional<model::ContactDetail> getContactDetail(const std::string& contactId) const override;
    bool canMoveContact(const std::string& childId, const std::string& newParentId) const override;
    void moveContact(const std::string& childId, const std::string& newParentId) override;
    bool canAddContact(const std::string& parentId, model::ContactNodeType type) const override;
    std::string addContact(const std::string& parentId, const model::ContactNodeData& data) override;
    void updateContact(const model::ContactNodeData& data) override;
    bool canRemoveContact(const std::string& contactId) const override;
    void removeContact(const std::string& contactId) override;

protected:
    virtual void init() override;

    // IContactServiceCallback overrides
    void onContactDirectoryReady() override;
    void onContactDirectoryLoadFailed(ucf::service::ContactDirectoryLoadError error) override;
    void onPersonContactsAdded(const ucf::service::model::PersonContactArray& persons) override;
    void onPersonContactsUpdated(const ucf::service::model::PersonContactArray& persons) override;
    void onPersonContactsRemoved(const std::vector<std::string>& contactIds) override;
    void onGroupContactsAdded(const ucf::service::model::GroupContactArray& groups) override;
    void onGroupContactsUpdated(const ucf::service::model::GroupContactArray& groups) override;
    void onGroupContactsRemoved(const std::vector<std::string>& contactIds) override;
    void onContactRelationsAdded(const ucf::service::model::ContactRelationArray& relations) override;
    void onContactRelationsUpdated(const ucf::service::model::ContactRelationArray& relations) override;
    void onContactRelationsRemoved(const std::vector<std::string>& relationIds) override;

private:
    std::shared_ptr<ucf::service::IContactService> lockService() const;
    void rebuildTreeFromService();
    // Returns true if the tree was just rebuilt; callers should skip the incremental apply.
    bool ensureTreeBuilt();
    ucf::service::model::IContactRelation::RelationType serviceRelationType() const;

    // Returns a shared snapshot of the current tree under the tree mutex (may be null).
    std::shared_ptr<model::ContactTree> snapshotTree() const;
    // Looks up a node by id in the given tree snapshot (null-safe).
    model::ContactTreeNodePtr findNode(const std::shared_ptr<model::ContactTree>& tree,
                                       const std::string& id) const;

    // Keeps only the groups whose type matches this slice; std::nullopt means the slice
    // carries no group type at all and the whole event should be dropped.
    std::optional<ucf::service::model::GroupContactArray>
        filterGroupsForSlice(const ucf::service::model::GroupContactArray& groups) const;
    // Keeps only the relations whose type matches this slice.
    ucf::service::model::ContactRelationArray
        filterRelationsForSlice(const ucf::service::model::ContactRelationArray& relations) const;

private:
    mutable std::mutex mTreeMutex;
    std::shared_ptr<model::ContactTree> mTree;

    model::RelationType mInterestedRelationType{model::RelationType::Folder};
};
}
