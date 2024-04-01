#include "ContactService/ContactModel.h"

#include "CoreFramework/ICoreFramework.h"
#include "ServiceCommonFile/ServiceLogger.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <set>
namespace model{
Contact::Contact(const std::string& id, const std::string& name)
    : mContactId(id)
    , mContactName(name)
{

}

bool Contact::operator==(const Contact& other)
{
    return this->mContactId == other.mContactId;
}

const std::string& Contact::getContactId() const
{
    return mContactId;
}

const std::string& Contact::getContactName() const
{
    return mContactName;
}

void Contact::setContactName(const std::string& name)
{
    mContactName = name;
}

const std::vector<std::string>& Contact::getTags() const
{
    return mContactTags;
}

void Contact::setTags(const std::vector<std::string>& tags)
{
    mContactTags = tags;
}

void Contact::addTags(const std::vector<std::string>& tags)
{

    std::set<std::string> tempSet;
    std::for_each(mContactTags.cbegin(), mContactTags.cend(), [&tempSet](const std::string& tag){
        tempSet.insert(tag);
    });

    std::for_each(tags.cbegin(), tags.cend(),[this, &tempSet](const std::string& tag){
        if (!tempSet.contains(tag))
        {
            mContactTags.push_back(tag);
            tempSet.insert(tag);
        }
    });
}

void Contact::removeTags(const std::vector<std::string>& tags)
{
    std::for_each(tags.cbegin(), tags.cend(),[this](const std::string& tag){
        mContactTags.erase(std::remove(mContactTags.begin(), mContactTags.end(), tag),mContactTags.end());
    });
}

ContactModel::ContactModel(std::weak_ptr<ICoreFramework> coreFramework)
:  mCoreFrameworkWPtr(coreFramework)
{
    SERVICE_LOG_DEBUG("create ContactModel, address:"  << this);

    mContacts.push_back(model::Contact("1234", "test name1"));
    mContacts.push_back(model::Contact("5678", "test name2"));
}

const std::vector<Contact>& ContactModel::getContacts() const
{
    std::scoped_lock loc(mContactMutex);
    return mContacts;
}

void ContactModel::setContacts(const std::vector<Contact>& contacts)
{
    std::scoped_lock loc(mContactMutex);
    mContacts = contacts;
}

void ContactModel::deleteContacts(const std::string& contactId)
{
    std::scoped_lock loc(mContactMutex);
    mContacts.erase(std::remove_if(mContacts.begin(), mContacts.end(),[contactId](const Contact& contact){
        return contactId == contact.getContactId();
    }),mContacts.end());
}

void ContactModel::addOrUpdateContact(const Contact& contact)
{
    std::scoped_lock loc(mContactMutex);
    if (auto iter = std::find(mContacts.begin(), mContacts.end(), contact); iter != mContacts.end())
    {
        *iter = contact;
    }
    else
    {
        mContacts.push_back(contact);
    }
}
}