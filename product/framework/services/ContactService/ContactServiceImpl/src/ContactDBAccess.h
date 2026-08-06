#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "ContactEntities.h"

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

// Database I/O only: marshals in-memory objects to/from DB rows. Holds no cache.
class ContactDBAccess
{
public:
    using LoadPersonsCallback   = std::function<void(const model::PersonContactArray&)>;
    using LoadGroupsCallback    = std::function<void(const model::GroupContactArray&)>;
    using LoadRelationsCallback = std::function<void(const model::ContactRelationArray&)>;

    explicit ContactDBAccess(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~ContactDBAccess() = default;
    ContactDBAccess(const ContactDBAccess&) = delete;
    ContactDBAccess(ContactDBAccess&&) = delete;
    ContactDBAccess& operator=(const ContactDBAccess&) = delete;
    ContactDBAccess& operator=(ContactDBAccess&&) = delete;

public:
    void setDatabaseId(const std::string& dbId);
    const std::string& getDatabaseId() const;

    // ===== Load (async) =====
    void loadPersonContacts(LoadPersonsCallback callback) const;
    void loadGroupContacts(LoadGroupsCallback callback) const;
    void loadContactRelations(LoadRelationsCallback callback) const;

    // ===== Persist (sync) =====
    void insertPersonContacts(const model::PersonContactArray& persons) const;
    void updatePersonContact(const model::IPersonContactPtr& person) const;
    void deletePersonContact(const std::string& contactId) const;

    void insertGroupContacts(const model::GroupContactArray& groups) const;
    void updateGroupContact(const model::IGroupContactPtr& group) const;
    void deleteGroupContact(const std::string& contactId) const;

    void insertContactRelations(const model::ContactRelationArray& relations) const;
    void updateContactRelation(const model::IContactRelationPtr& relation) const;
    void deleteContactRelation(const std::string& relationId) const;

private:
    std::string resolveDatabaseId() const;

private:
    const ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    std::string mDatabaseId;  // Set by Model on DB bind; otherwise falls back to ClientInfoService.
};

} // namespace ucf::service
