#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ContactEntities.h"
#include "ContactNotificationSink.h"

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}
namespace ucf::adapter {
    class ContactAdapter;
}

namespace ucf::service {

class ContactModel;

// Business orchestration layer; today a thin forwarder onto ContactModel.
class ContactManager final
{
public:
    explicit ContactManager(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~ContactManager();
    ContactManager(const ContactManager&) = delete;
    ContactManager(ContactManager&&) = delete;
    ContactManager& operator=(const ContactManager&) = delete;
    ContactManager& operator=(ContactManager&&) = delete;

public:
    // ===== Read =====
    model::PersonContactArray   getPersonContactList() const;
    model::GroupContactArray    getGroupContactList() const;
    model::ContactRelationArray getContactRelations() const;
    model::IPersonContactPtr    getPersonContact(const std::string& contactId) const;
    model::IGroupContactPtr     getGroupContact(const std::string& contactId) const;

    // ===== Batch write =====
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

    // Forwarded to the Model; Manager keeps a weak_ptr copy for future business-level events.
    void setNotificationSink(std::weak_ptr<IContactNotificationSink> sink);

private:
    const ucf::framework::ICoreFrameworkWPtr      mCoreFrameworkWPtr;
    const std::unique_ptr<ContactModel>           mContactModel;
    const std::unique_ptr<ucf::adapter::ContactAdapter> mContactAdapter;

    std::weak_ptr<IContactNotificationSink> mNotificationSink;
};

} // namespace ucf::service
