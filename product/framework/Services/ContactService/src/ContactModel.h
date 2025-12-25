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
    std::vector<model::IPersonContactPtr> getPersonContacts() const;
    model::IPersonContactPtr getPersonContact(const std::string& contactId) const;
    void addPersonContact(const model::PersonContactPtr& contact);
    void deletePersonContacts(const std::initializer_list<std::string>& contactId);

    std::vector<model::IGroupContactPtr> getGroupContacts() const;
    model::IGroupContactPtr getGroupContact(const std::string& contactId) const;
    void addGroupContact(const model::GroupContactPtr& contact);
    void deleteGroupContacts(const std::initializer_list<std::string>& contactId);

    std::vector<model::IContactRelationPtr> getContactRelations() const;
    void addContactRelation(const model::ContactRelationPtr& relation);
    void deleteContactRelations(const std::initializer_list<std::string>& childIds);
    void modifyContactRelation(const std::string& childId, const std::string& newParentId);
private:
    mutable std::mutex mContactMutex;
    std::map<std::string, model::PersonContactPtr> mPersonContacts;
    std::map<std::string, model::GroupContactPtr> mGroupContacts;
    std::vector<model::ContactRelationPtr> mContactRelations;

    ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    // std::shared_ptr<ucf::utilities::database::IDatabaseWrapper> mContactDatabase;
};
}