#include "ContactListViewModel.h"

#include <ucf/Services/ContactService/IContactService.h>
#include <ucf/Services/ContactService/IContactEntities.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>

#include <commonHead/viewModels/ContactListViewModel/ContactListModel.h>


namespace commonHead::viewModels{
std::shared_ptr<IContactListViewModel> IContactListViewModel::createInstance(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<ContactListViewModel>(commonHeadFramework);
}

ContactListViewModel::ContactListViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : IContactListViewModel(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create ContactListViewModel");
}

std::string ContactListViewModel::getViewModelName() const
{
    return "ContactListViewModel";
}

void ContactListViewModel::init()
{

}

std::vector<commonHead::viewModels::model::Contact> ContactListViewModel::getContactList() const
{
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto contactService = serviceLocator->getContactService().lock())
            {
                auto contactList = contactService->getPersonContactList();
                std::vector<commonHead::viewModels::model::Contact> contacts;
                for(auto conact: contactList)
                {
                    contacts.emplace_back(commonHead::viewModels::model::Contact{conact->getContactId()});
                }
                return contacts;
            }
        }
    }
    return {};
}
}