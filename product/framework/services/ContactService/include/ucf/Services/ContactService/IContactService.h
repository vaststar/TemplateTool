#pragma once

#include <memory>
#include <string>
#include <vector>

#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>
#include <ucf/Services/ServiceDeclaration/IService.h>
#include <ucf/Services/ContactService/IContactServiceCallback.h>

namespace ucf::framework {
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service {

class SERVICE_EXPORT IContactService:
    public IService,
    public virtual ucf::utilities::INotificationHelper<IContactServiceCallback>
{
public:
    IContactService() = default;
    IContactService(const IContactService&) = delete;
    IContactService(IContactService&&) = delete;
    IContactService& operator=(const IContactService&) = delete;
    IContactService& operator=(IContactService&&) = delete;
    virtual ~IContactService() = default;

public:
    // ===== Read =====
    [[nodiscard]] virtual model::PersonContactArray   getPersonContactList() const = 0;
    [[nodiscard]] virtual model::GroupContactArray    getGroupContactList() const = 0;
    [[nodiscard]] virtual model::ContactRelationArray getContactRelations() const = 0;
    [[nodiscard]] virtual model::IPersonContactPtr    getPersonContact(const std::string& contactId) const = 0;
    [[nodiscard]] virtual model::IGroupContactPtr     getGroupContact(const std::string& contactId) const = 0;

    // ===== Batch write =====
    virtual void addPersonContacts(const model::PersonContactArray& persons)    = 0;
    virtual void updatePersonContacts(const model::PersonContactArray& persons) = 0;
    virtual void removePersonContacts(const std::vector<std::string>& contactIds) = 0;

    virtual void addGroupContacts(const model::GroupContactArray& groups)    = 0;
    virtual void updateGroupContacts(const model::GroupContactArray& groups) = 0;
    virtual void removeGroupContacts(const std::vector<std::string>& contactIds) = 0;

    virtual void addContactRelations(const model::ContactRelationArray& relations)    = 0;
    virtual void updateContactRelations(const model::ContactRelationArray& relations) = 0;
    virtual void removeContactRelations(const std::vector<std::string>& childIds)     = 0;

    // ===== Lifecycle =====
    // Trigger loading the contact directory from the database. Requires the database to be
    // bound first; idempotent while Loading / Ready; retries when Failed.
    virtual void loadContactDirectory() = 0;

    // True once loadContactDirectory has completed successfully; lets late subscribers skip
    // the missed onContactDirectoryReady callback.
    [[nodiscard]] virtual bool isContactDirectoryReady() const = 0;

    static std::shared_ptr<IContactService> createInstance(ucf::framework::ICoreFrameworkWPtr coreFramework);
};

} // namespace ucf::service

