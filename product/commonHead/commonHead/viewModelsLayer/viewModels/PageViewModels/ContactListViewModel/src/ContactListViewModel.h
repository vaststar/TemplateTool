#pragma once

#include <memory>
#include <mutex>
#include <string>

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
    bool isContactDirectoryReady() const override;
    std::string getCurrentContactId() const override;
    void selectContact(const std::string& contactId) override;
    bool canMoveContact(const std::string& childId, const std::string& newParentId) const override;
    void moveContact(const std::string& childId, const std::string& newParentId) override;

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
    // Returns true when the tree was just rebuilt (because it was null); callers should
    // skip the incremental apply because the rebuilt snapshot already contains the delta.
    bool ensureTreeBuilt();

    // Returns the service-level RelationType this VM is interested in, derived once from
    // mInterestedRelationType.
    ucf::service::model::IContactRelation::RelationType serviceRelationType() const;

private:
    mutable std::mutex mTreeMutex;
    std::shared_ptr<model::ContactTree> mTree;

    mutable std::mutex mSelectionMutex;
    std::string mCurrentContactId;

    // The relation slice this VM instance is bound to. Today the factory only constructs
    // Department VMs (matches legacy behaviour); a future factory overload will let UI
    // construct VMs for other slices without touching this class.
    model::RelationType mInterestedRelationType{model::RelationType::Department};
};
}
