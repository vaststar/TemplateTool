#pragma once

#include <string>
#include <vector>

#include <ucf/Services/ContactService/IContactEntities.h>

namespace ucf::service {

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
    // Contact directory is ready (DB initialized & loaded into memory)
    virtual void onContactDirectoryReady() {}

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

