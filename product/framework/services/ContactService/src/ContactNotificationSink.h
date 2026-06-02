#pragma once

#include <string>
#include <vector>

#include <ucf/Services/ContactService/IContactServiceCallback.h>
#include "ContactEntities.h"

namespace ucf::service {

// Origin of a change; reserved for future routing decisions.
enum class ContactNotificationSource {
    Local,
    Remote,
    DbLoad,
    Reconcile,
};

// Internal sink: Model invokes it after a state change; Service translates to outward callbacks.
class IContactNotificationSink
{
public:
    virtual ~IContactNotificationSink() = default;

    virtual void onPersonContactsAdded   (const model::PersonContactArray& persons,    ContactNotificationSource src) = 0;
    virtual void onPersonContactsUpdated (const model::PersonContactArray& persons,    ContactNotificationSource src) = 0;
    virtual void onPersonContactsRemoved (const std::vector<std::string>& contactIds,  ContactNotificationSource src) = 0;

    virtual void onGroupContactsAdded    (const model::GroupContactArray& groups,      ContactNotificationSource src) = 0;
    virtual void onGroupContactsUpdated  (const model::GroupContactArray& groups,      ContactNotificationSource src) = 0;
    virtual void onGroupContactsRemoved  (const std::vector<std::string>& contactIds,  ContactNotificationSource src) = 0;

    virtual void onContactRelationsAdded   (const model::ContactRelationArray& relations, ContactNotificationSource src) = 0;
    virtual void onContactRelationsUpdated (const model::ContactRelationArray& relations, ContactNotificationSource src) = 0;
    virtual void onContactRelationsRemoved (const std::vector<std::string>& childIds,     ContactNotificationSource src) = 0;

    virtual void onDirectoryLoaded() {}
    virtual void onDirectoryLoadFailed(ContactDirectoryLoadError /*error*/) {}
};

} // namespace ucf::service
