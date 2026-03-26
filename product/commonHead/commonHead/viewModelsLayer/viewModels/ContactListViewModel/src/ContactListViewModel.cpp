#include "ContactListViewModel.h"

#include <ucf/Services/ContactService/IContactService.h>
#include <ucf/Services/ContactService/IContactEntities.h>

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ServiceLocator/IServiceLocator.h>

#include "ContactListModel.h"


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
    buildContactTree();
}

void ContactListViewModel::buildContactTree()
{
    if (auto commonHeadFramework = getCommonHeadFramework().lock())
    {
        if (auto serviceLocator = commonHeadFramework->getServiceLocator())
        {
            if (auto contactService = serviceLocator->getContactService().lock())
            {
                std::vector<ucf::service::model::IContactPtr> allContacts;
                auto allPersonContacts = contactService->getPersonContactList();
                auto allGroupContacts = contactService->getGroupContactList();
                allContacts.insert(allContacts.end(), allPersonContacts.begin(), allPersonContacts.end());
                allContacts.insert(allContacts.end(), allGroupContacts.begin(), allGroupContacts.end());
                auto allRelations = contactService->getContactRelations();

                mContactTreePtr = std::make_shared<model::ContactTree>(allContacts, allRelations);
            }
        }
    }
}

model::ContactTreePtr ContactListViewModel::getContactList() const
{
    return mContactTreePtr;
}
}