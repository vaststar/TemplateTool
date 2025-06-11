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
    ContactListViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
    virtual std::string getViewModelName() const override;
    virtual std::vector<commonHead::viewModels::model::Contact> getContactList() const override;

private:
    commonHead::ICommonHeadFrameworkWptr mCommonHeadFrameworkWptr;
};
}