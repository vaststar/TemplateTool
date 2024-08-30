
#include <algorithm>
#include <iterator>
#include <map>
#include <set>


#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/Services/ServiceCommonFile/ServiceLogger.h>

#include <ucf/Services/ContactService/Contact.h>
#include "ContactModel.h"

namespace ucf::service{


ContactModel::ContactModel(ucf::framework::ICoreFrameworkWPtr coreFramework)
:  mCoreFrameworkWPtr(coreFramework)
{
    SERVICE_LOG_DEBUG("create ContactModel, address:"  << this);

    mContacts.push_back(model::PersonContact("1234"));
    mContacts.push_back(model::PersonContact("5678"));
}

void ContactModel::initDataBase()
{

}

const std::vector<model::PersonContact>& ContactModel::getContacts() const
{
    std::scoped_lock loc(mContactMutex);
    return mContacts;
}

void ContactModel::setContacts(const std::vector<model::PersonContact>& contacts)
{
    // std::scoped_lock loc(mContactMutex);
    // mContacts = contacts;
}

void ContactModel::deleteContacts(const std::string& contactId)
{
    // std::scoped_lock loc(mContactMutex);
    // mContacts.erase(std::remove_if(mContacts.cbegin(), mContacts.cend(),[contactId](const model::PersonContact& contact){
    //     return contactId == contact.getContactId();
    // }),mContacts.cend());
}

void ContactModel::addOrUpdateContact(const model::PersonContact& contact)
{
    // std::scoped_lock loc(mContactMutex);
    // if (auto iter = std::find(mContacts.begin(), mContacts.end(), contact); iter != mContacts.end())
    // {
    //     //*iter.getContactId() = contact;
    // }
    // else
    // {
    //     mContacts.push_back(contact);
    // }
}
}