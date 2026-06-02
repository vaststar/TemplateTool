#pragma once

#include <string>
#include <vector>

#include <ucf/Services/ContactService/IContactEntities.h>

namespace ucf::service {

// Reason for a directory load failure; loadContactDirectory() can be retried after a failure.
enum class ContactDirectoryLoadError
{
    Unknown,
    DatabaseNotBound,
    DatabaseReadFailed,
};

class IContactServiceCallback
{
public:
    IContactServiceCallback() = default;
    IContactServiceCallback(const IContactServiceCallback&) = delete;
    IContactServiceCallback(IContactServiceCallback&&) = delete;
    IContactServiceCallback& operator=(const IContactServiceCallback&) = delete;
    IContactServiceCallback& operator=(IContactServiceCallback&&) = delete;
    virtual ~IContactServiceCallback() = default;

public:
    // Directory load finished successfully.
    virtual void onContactDirectoryReady() {}

    // Directory load failed; retry via loadContactDirectory().
    virtual void onContactDirectoryLoadFailed(ContactDirectoryLoadError /*error*/) {}

    // ===== Person contacts =====
    virtual void onPersonContactsAdded(const model::PersonContactArray& /*persons*/) {}
    virtual void onPersonContactsUpdated(const model::PersonContactArray& /*persons*/) {}
    virtual void onPersonContactsRemoved(const std::vector<std::string>& /*contactIds*/) {}

    // ===== Group contacts =====
    virtual void onGroupContactsAdded(const model::GroupContactArray& /*groups*/) {}
    virtual void onGroupContactsUpdated(const model::GroupContactArray& /*groups*/) {}
    virtual void onGroupContactsRemoved(const std::vector<std::string>& /*contactIds*/) {}

    // ===== Relations =====
    virtual void onContactRelationsAdded(const model::ContactRelationArray& /*relations*/) {}
    virtual void onContactRelationsUpdated(const model::ContactRelationArray& /*relations*/) {}
    virtual void onContactRelationsRemoved(const std::vector<std::string>& /*childIds*/) {}
};

} // namespace ucf::service

