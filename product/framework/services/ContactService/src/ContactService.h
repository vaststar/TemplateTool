#pragma once

#include <memory>
#include <vector>
#include <typeindex>

#include <ucf/Utilities/NotificationHelper/NotificationHelper.h>
#include <ucf/CoreFramework/CoreFrameworkCallbackDefault.h>
#include <ucf/Services/ContactService/IContactService.h>
#include <ucf/Services/DataWarehouseService/IDataWarehouseServiceCallback.h>

#include "ContactNotificationSink.h"

namespace ucf::service {

class SERVICE_EXPORT ContactService final
    : public virtual IContactService
    , public virtual ucf::utilities::NotificationHelper<IContactServiceCallback>
    , public ucf::framework::CoreFrameworkCallbackDefault
    , public ucf::service::IDataWarehouseServiceCallback
    , public IContactNotificationSink
    , public std::enable_shared_from_this<ContactService>
{
public:
    explicit ContactService(ucf::framework::ICoreFrameworkWPtr coreFramework);
    virtual ~ContactService();
    ContactService(const ContactService&) = delete;
    ContactService(ContactService&&) = delete;
    ContactService& operator=(const ContactService&) = delete;
    ContactService& operator=(ContactService&&) = delete;

public:
    // IService
    virtual std::string getServiceName() const override;
    virtual std::vector<std::type_index> dependencies() const override;

    // ICoreFrameworkCallback
    virtual void onServiceInitialized() override;
    virtual void onCoreFrameworkExit() override;

    // IDataWarehouseServiceCallback
    virtual void OnDatabaseInitialized(const std::string& dbId) override;

    // IContactService - Read
    virtual model::PersonContactArray   getPersonContactList() const override;
    virtual model::GroupContactArray    getGroupContactList(
        std::optional<model::IGroupContact::GroupType> groupType = std::nullopt) const override;
    virtual model::ContactRelationArray getContactRelations(
        std::optional<model::IContactRelation::RelationType> relationType = std::nullopt) const override;
    virtual model::IPersonContactPtr    getPersonContact(const std::string& contactId) const override;
    virtual model::IGroupContactPtr     getGroupContact(const std::string& contactId) const override;

    // IContactService - Batch write
    virtual void addPersonContacts(const model::PersonContactArray& persons) override;
    virtual void updatePersonContacts(const model::PersonContactArray& persons) override;
    virtual void removePersonContacts(const std::vector<std::string>& contactIds) override;

    virtual void addGroupContacts(const model::GroupContactArray& groups) override;
    virtual void updateGroupContacts(const model::GroupContactArray& groups) override;
    virtual void removeGroupContacts(const std::vector<std::string>& contactIds) override;

    virtual void addContactRelations(const model::ContactRelationArray& relations) override;
    virtual void updateContactRelations(const model::ContactRelationArray& relations) override;
    virtual void removeContactRelations(const std::vector<std::string>& relationIds) override;

    // IContactService - Lifecycle
    virtual void loadContactDirectory() override;
    virtual bool isContactDirectoryReady() const override;

protected:
    // IService
    virtual void initService() override;

private:
    // IContactNotificationSink — translation layer from internal changes to outward
    // notifications; not exposed externally.
    virtual void onPersonContactsAdded   (const model::PersonContactArray& persons,    ContactNotificationSource src) override;
    virtual void onPersonContactsUpdated (const model::PersonContactArray& persons,    ContactNotificationSource src) override;
    virtual void onPersonContactsRemoved (const std::vector<std::string>& contactIds,  ContactNotificationSource src) override;
    virtual void onGroupContactsAdded    (const model::GroupContactArray& groups,      ContactNotificationSource src) override;
    virtual void onGroupContactsUpdated  (const model::GroupContactArray& groups,      ContactNotificationSource src) override;
    virtual void onGroupContactsRemoved  (const std::vector<std::string>& contactIds,  ContactNotificationSource src) override;
    virtual void onContactRelationsAdded   (const model::ContactRelationArray& relations,    ContactNotificationSource src) override;
    virtual void onContactRelationsUpdated (const model::ContactRelationArray& relations,    ContactNotificationSource src) override;
    virtual void onContactRelationsRemoved (const std::vector<std::string>& relationIds, ContactNotificationSource src) override;
    virtual void onDirectoryLoaded() override;
    virtual void onDirectoryLoadFailed(ContactDirectoryLoadError error) override;

private:
    class DataPrivate;
    std::unique_ptr<DataPrivate> mDataPrivate;
};

} // namespace ucf::service
