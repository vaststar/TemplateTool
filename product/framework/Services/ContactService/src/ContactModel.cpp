
#include <algorithm>
#include <iterator>
#include <map>
#include <set>


#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ServiceCommonFile/ServiceLogger.h>

#include "ContactModel.h"

namespace ucf::service{


ContactModel::ContactModel(ucf::framework::ICoreFrameworkWPtr coreFramework)
:  mCoreFrameworkWPtr(coreFramework)
{
    SERVICE_LOG_DEBUG("create ContactModel, address:"  << this);

    mContacts.push_back(model::Contact("1234", "test name1"));
    mContacts.push_back(model::Contact("5678", "test name2"));
}

void ContactModel::initDataBase()
{

}

const std::vector<model::Contact>& ContactModel::getContacts() const
{
    std::scoped_lock loc(mContactMutex);
    return mContacts;
}

void ContactModel::setContacts(const std::vector<model::Contact>& contacts)
{
    std::scoped_lock loc(mContactMutex);
    mContacts = contacts;
}

void ContactModel::deleteContacts(const std::string& contactId)
{
    std::scoped_lock loc(mContactMutex);
    mContacts.erase(std::remove_if(mContacts.begin(), mContacts.end(),[contactId](const model::Contact& contact){
        return contactId == contact.getContactId();
    }),mContacts.end());
}

void ContactModel::addOrUpdateContact(const model::Contact& contact)
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