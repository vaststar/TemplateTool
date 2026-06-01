#pragma once

#include <memory>
#include <mutex>

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

protected:
    virtual void init() override;

    // IContactServiceCallback overrides
    void onContactDirectoryReady() override;
    void onPersonContactsAdded(const ucf::service::model::PersonContactArray& persons) override;
    void onPersonContactsUpdated(const ucf::service::model::PersonContactArray& persons) override;
    void onPersonContactsRemoved(const std::vector<std::string>& contactIds) override;
    void onGroupContactsAdded(const ucf::service::model::GroupContactArray& groups) override;
    void onGroupContactsUpdated(const ucf::service::model::GroupContactArray& groups) override;
    void onGroupContactsRemoved(const std::vector<std::string>& contactIds) override;
    void onContactRelationsAdded(const ucf::service::model::ContactRelationArray& relations) override;
    void onContactRelationsUpdated(const ucf::service::model::ContactRelationArray& relations) override;
    void onContactRelationsRemoved(const std::vector<std::string>& childIds) override;

private:
    std::shared_ptr<ucf::service::IContactService> lockService() const;
    void rebuildTreeFromService();

private:
    mutable std::mutex mTreeMutex;
    std::shared_ptr<model::ContactTree> mTree;
};
}
