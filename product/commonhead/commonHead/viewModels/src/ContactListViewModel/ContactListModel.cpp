#include <commonHead/viewModels/ContactListViewModel/ContactListModel.h>

namespace commonHead::viewModels::model{

Contact::Contact(const std::string& contactId)
    : mContactId(contactId)
{

}

std::string Contact::getContactId() const
{
    return mContactId;

}

std::string Contact::getContactName() const
{
    return mContactName;
}

void Contact::setContactName(const std::string& name)
{
    mContactName = name;
}

std::string Contact::getUpperId() const
{
    return mUpperId;
}

void Contact::setUpperId(const std::string& upperId)
{
    mUpperId = upperId;
}

std::vector<std::string> Contact::getLowerIds() const
{
    return mLowerIds;
}

void Contact::setLowerIds(const std::vector<std::string>& lowerIds)
{
    mLowerIds = lowerIds;
}
}