#include "ContactListViewModel.h"

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/ContactService/IContactService.h>
#include <ucf/ContactService/ContactModel.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

#include <commonHead/viewModels/ContactListViewModel/ContactListModel.h>


namespace commonHead::viewModels{
std::shared_ptr<IContactListViewModel> IContactListViewModel::CreateInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<ContactListViewModel>(commonHeadFramework);
}

ContactListViewModel::ContactListViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : mCommonHeadFrameworkWptr(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create ContactListViewModel");
}

std::string ContactListViewModel::getViewModelName() const
{
    return "ContactListViewModel";
}

std::vector<commonHead::viewModels::model::Contact> ContactListViewModel::getContactList() const
{
    if (auto commonHeadFramework = mCommonHeadFrameworkWptr.lock())
    {
        if (auto coreFramework = commonHeadFramework->getCoreFramework().lock())
        {
            if (auto contactService  = coreFramework->getService<ucf::IContactService>().lock())
            {
                auto contactList = contactService->getContactList();
                std::vector<commonHead::viewModels::model::Contact> contacts;
                for(auto conact: contactList)
                {
                    contacts.emplace_back(commonHead::viewModels::model::Contact{conact.getContactId()});
                }
                return contacts;
            }
        }
    }
    return {};
}
}