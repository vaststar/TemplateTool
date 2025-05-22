#include "ContactModel.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <set>


#include <ucf/CoreFramework/ICoreFramework.h>

#include <ucf/Services/ContactService/Contact.h>
#include "ContactServiceLogger.h"
// #include <ucf/Utilities/DatabaseUtils/DatabaseWrapper/IDatabaseWrapper.h>

namespace ucf::service{


ContactModel::ContactModel(ucf::framework::ICoreFrameworkWPtr coreFramework)
:  mCoreFrameworkWPtr(coreFramework)
{
    SERVICE_LOG_DEBUG("create ContactModel, address:"  << this);

    addPersonContact(std::make_unique<model::PersonContact>("1234"));
    addPersonContact(std::make_unique<model::PersonContact>("5678"));
}

void ContactModel::initDatabase()
{
    // ucf::utilities::database::SqliteDatabaseConfig config{"test_database.db"};
    // mContactDatabase = ucf::utilities::database::IDatabaseWrapper::createSqliteDatabase(config);
    // mContactDatabase->open();
}

std::vector<model::PersonContact> ContactModel::getPersonContacts() const
{
    std::vector<model::PersonContact> results;
    std::scoped_lock loc(mContactMutex);
    std::transform(mPersonContacts.cbegin(), mPersonContacts.cend(), std::back_inserter(results), [](const auto& contactPair) {
        return *contactPair.second;
    });
    return results;
}

void ContactModel::deletePersonContacts(const std::initializer_list<std::string>& contactIds)
{
    std::scoped_lock loc(mContactMutex);
    for (const auto& contactId : contactIds)
    {
        if (auto iter = mPersonContacts.find(contactId); iter != mPersonContacts.end())
        {
            mPersonContacts.erase(iter);
        }
    }
}

void ContactModel::addPersonContact(std::unique_ptr<model::PersonContact>&& contact)
{
    std::scoped_lock loc(mContactMutex);
    if (auto iter = mPersonContacts.find(contact->getContactId()); iter != mPersonContacts.end())
    {
        SERVICE_LOG_DEBUG("Contact already exists, contactId:" << contact->getContactId());
        return;
    }
    mPersonContacts[contact->getContactId()] = std::move(contact);
}

std::optional<model::PersonContact> ContactModel::getPersonContact(const std::string& contactId) const
{
    std::scoped_lock loc(mContactMutex);
    if (auto iter = mPersonContacts.find(contactId); iter != mPersonContacts.end())
    {
        return *iter->second;
    }
    return std::nullopt;
}
}