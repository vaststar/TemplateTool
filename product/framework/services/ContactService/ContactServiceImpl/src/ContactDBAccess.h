#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <ucf/services/ContactService/IContactServiceCallback.h>

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
    ContactWriteError insertPersonContacts(const model::PersonContactArray& persons) const;
    ContactWriteError updatePersonContact(const model::IPersonContactPtr& person) const;
    ContactWriteError deletePersonContact(const std::string& contactId) const;

    ContactWriteError insertGroupContacts(const model::GroupContactArray& groups) const;
    ContactWriteError updateGroupContact(const model::IGroupContactPtr& group) const;
    ContactWriteError deleteGroupContact(const std::string& contactId) const;

    ContactWriteError insertContactRelations(const model::ContactRelationArray& relations) const;
    ContactWriteError updateContactRelation(const model::IContactRelationPtr& relation) const;
    ContactWriteError deleteContactRelation(const std::string& relationId) const;

private:
    std::string resolveDatabaseId() const;

private:
    const ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    std::string mDatabaseId;  // Set by Model on DB bind; otherwise falls back to ClientInfoService.
};

} // namespace ucf::service
