#include "ContactListViewModel/ContactListViewModel.h"
#include "CoreFramework/ICoreFramework.h"
#include "CommonHeadCommonFile/CommonHeadLogger.h"
#include "CommonHeadFramework/ICommonHeadFramework.h"
#include "ContactService/IContactService.h"
#include "ContactService/ContactModel.h"
#include "ContactListViewModel/ContactListModel.h"


namespace CommonHead::ViewModels{
std::shared_ptr<IContactListViewModel> IContactListViewModel::CreateInstance(ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<ContactListViewModel>(commonHeadFramework);
}

ContactListViewModel::ContactListViewModel(ICommonHeadFrameworkWptr commonHeadFramework)
    : mCommonHeadFrameworkWptr(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create ContactListViewModel");
}

std::string ContactListViewModel::getViewModelName() const
{
    return "ContactListViewModel";
}

std::vector<CommonHead::ViewModels::Contact> ContactListViewModel::getContactList() const
{
    if (auto commonHeadFramework = mCommonHeadFrameworkWptr.lock())
    {
        if (auto coreFramework = commonHeadFramework->getCoreFramework().lock())
        {
            if (auto contactService  = coreFramework->getService<IContactService>().lock())
            {
                auto contactList = contactService->getContactList();
                std::vector<CommonHead::ViewModels::Contact> contacts;
                for(auto conact: contactList)
                {
                    contacts.emplace_back(CommonHead::ViewModels::Contact{conact.getContactId()});
                }
                return contacts;
            }
        }
    }
    return {};
}
}