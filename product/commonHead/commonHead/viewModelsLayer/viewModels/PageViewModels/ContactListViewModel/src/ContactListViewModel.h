#pragma once

#include <memory>

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/ContactListViewModel/IContactListViewModel.h>

namespace commonHead::viewModels{
class ContactListViewModel: public virtual IContactListViewModel, 
                            public virtual commonHead::utilities::VMNotificationHelper<IContactListViewModelCallback>,
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
private:
    void buildContactTree();
    model::ContactTreePtr mContactTreePtr;
};
}