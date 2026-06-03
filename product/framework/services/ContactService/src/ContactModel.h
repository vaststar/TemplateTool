#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "ContactEntities.h"
#include "ContactNotificationSink.h"

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class ContactDBAccess;

// In-memory contact view coordinated with database persistence.
class ContactModel
{
public:
    explicit ContactModel(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~ContactModel();
    ContactModel(const ContactModel&) = delete;
    ContactModel(ContactModel&&) = delete;
    ContactModel& operator=(const ContactModel&) = delete;
    ContactModel& operator=(ContactModel&&) = delete;

public:
    // ===== Read =====
    model::PersonContactArray   getPersonContacts() const;
    model::GroupContactArray    getGroupContacts() const;
    model::ContactRelationArray getContactRelations() const;
    model::IPersonContactPtr    getPersonContact(const std::string& contactId) const;
    model::IGroupContactPtr     getGroupContact(const std::string& contactId) const;

    // ===== Batch write: memory first then DB; returns items that actually took effect =====
    model::PersonContactArray addPersonContacts(const model::PersonContactArray& persons);
    model::PersonContactArray updatePersonContacts(const model::PersonContactArray& persons);
    std::vector<std::string>  removePersonContacts(const std::vector<std::string>& contactIds);

    model::GroupContactArray addGroupContacts(const model::GroupContactArray& groups);
    model::GroupContactArray updateGroupContacts(const model::GroupContactArray& groups);
    std::vector<std::string> removeGroupContacts(const std::vector<std::string>& contactIds);

    model::ContactRelationArray addContactRelations(const model::ContactRelationArray& relations);
    model::ContactRelationArray updateContactRelations(const model::ContactRelationArray& relations);
    std::vector<std::string>    removeContactRelations(const std::vector<std::string>& childIds);

    // ===== Lifecycle =====
    void bindDatabase(const std::string& databaseId);
    void loadContactDirectory();
    bool isContactDirectoryReady() const;

    // Sink is held as weak_ptr; safe for async DB callbacks arriving after Service teardown.
    void setNotificationSink(std::weak_ptr<IContactNotificationSink> sink);

private:
    // Memory-only primitives (do not touch the DB).
    model::PersonContactArray   addPersonContactsInMemory(const model::PersonContactArray& persons);
    model::PersonContactArray   updatePersonContactsInMemory(const model::PersonContactArray& persons);
    std::vector<std::string>    removePersonContactsInMemory(const std::vector<std::string>& contactIds);
    model::GroupContactArray    addGroupContactsInMemory(const model::GroupContactArray& groups);
    model::GroupContactArray    updateGroupContactsInMemory(const model::GroupContactArray& groups);
    std::vector<std::string>    removeGroupContactsInMemory(const std::vector<std::string>& contactIds);
    model::ContactRelationArray addContactRelationsInMemory(const model::ContactRelationArray& relations);
    model::ContactRelationArray updateContactRelationsInMemory(const model::ContactRelationArray& relations);
    std::vector<std::string>    removeContactRelationsInMemory(const std::vector<std::string>& childIds);

    // Single exit points for load completion / failure (fires exactly once per load).
    void finishLoadSuccess();
    void finishLoadFailure(ContactDirectoryLoadError error);

    enum class LoadStage : std::uint8_t {
        Uninit,
        DbBound,
        Loading,
        Ready,
        Failed,
    };

private:
    mutable std::mutex mMutex;
    std::unordered_map<std::string, std::shared_ptr<model::PersonContact>>   mPersonContacts;
    std::unordered_map<std::string, std::shared_ptr<model::GroupContact>>    mGroupContacts;
    std::unordered_map<std::string, std::shared_ptr<model::ContactRelation>> mContactRelations;  // key = childId

    const std::unique_ptr<ContactDBAccess> mContactDBAccess;

    std::weak_ptr<IContactNotificationSink> mNotificationSink;
    std::atomic<LoadStage> mLoadStage{LoadStage::Uninit};
    // Set when loadContactDirectory() is called before bindDatabase(); bindDatabase()
    // then auto-promotes the load once the DB id has been set. Lets callers be order
    // independent: registering and calling load before the DB is ready is safe.
    std::atomic<bool>      mLoadPending{false};
};

} // namespace ucf::service
