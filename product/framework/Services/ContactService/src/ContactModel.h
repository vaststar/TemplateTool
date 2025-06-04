#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <map>
#include <initializer_list>
#include <optional>

#include "ContactEntities.h"

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}
namespace ucf::service{

class ContactModel
{
public:
    explicit ContactModel(ucf::framework::ICoreFrameworkWPtr coreFramework);
    
    ContactModel(const ContactModel&) = delete;
    ContactModel(ContactModel&&) = delete;
    ContactModel& operator=(const ContactModel&) = delete;
    ContactModel& operator=(ContactModel&&) = delete;

public:
    void initDatabase();
    std::vector<model::IPersonContactPtr> getPersonContacts() const;
    model::IPersonContactPtr getPersonContact(const std::string& contactId) const;

    void addPersonContact(const model::PersonContactPtr& contact);
    void deletePersonContacts(const std::initializer_list<std::string>& contactId);
private:
    mutable std::mutex mContactMutex;
    std::map<std::string, model::PersonContactPtr> mPersonContacts;
    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    // std::shared_ptr<ucf::utilities::database::IDatabaseWrapper> mContactDatabase;
};
}