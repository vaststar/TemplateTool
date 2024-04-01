#pragma once
#include <memory>
#include "ContactListViewModel/IContactListViewModel.h"

namespace CommonHead::ViewModels{
class ContactListViewModel: public IContactListViewModel, 
                            public std::enable_shared_from_this<ContactListViewModel>
{
public:
    ContactListViewModel(ICommonHeadFrameworkWptr commonHeadFramework);
    virtual std::string getViewModelName() const override;
    virtual std::vector<CommonHead::ViewModels::Contact> getContactList() const override;

private:
    ICommonHeadFrameworkWptr mCommonHeadFrameworkWptr;
};
}