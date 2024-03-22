#include "ContactListViewModel/ContactListModel.h"

namespace CommonHead::ViewModels{

Contact::Contact(const std::string& contactId)
    : mContactId(contactId)
{

}

std::string Contact::getContactId() const
{
    return mContactId;
}

}