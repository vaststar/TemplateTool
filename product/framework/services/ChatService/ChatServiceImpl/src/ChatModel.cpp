#include "ChatModel.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "ChatDBAccess.h"
#include "ChatUtils.h"

namespace ucf::service::chat {
namespace {

constexpr std::size_t MaxGroupNameBytes = 96;

void require(bool accepted, const char* error)
{
    if (!accepted)
    {
        throw std::runtime_error(error);
    }
}

std::string groupName(std::string value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    const auto last = value.find_last_not_of(" \t\r\n");
    if (first != std::string::npos)
    {
        value = value.substr(first, last - first + 1);
    }
    require(validText(value, MaxGroupNameBytes) && value.find_first_of("\r\n") == std::string::npos,
            "Enter a valid group name.");
    return value;
}

} // namespace

ChatModel::ChatModel(std::shared_ptr<IDataWarehouseService> database, std::string dbId)
    : mDatabase(std::make_unique<ChatDBAccess>(std::move(database), std::move(dbId)))
{
}

ChatModel::~ChatModel() = default;

bool ChatModel::ready() const
{
    return mDatabase->ready();
}

Snapshot ChatModel::snapshot(const std::string& id) const
{
    Snapshot result;
    if (!ready())
    {
        return result;
    }
    result.ready = true;
    result.groups = mDatabase->groups();
    result.conversations = mDatabase->conversations();
    result.directoryRelations = mDatabase->directoryRelations();
    if (!id.empty())
    {
        result.messages = mDatabase->messages(id);
    }
    return result;
}

Conversation ChatModel::conversation(const std::string& id) const
{
    const auto conversations = mDatabase->conversations();
    const auto existing = std::ranges::find(conversations, id, &Conversation::id);
    require(existing != conversations.end(), "Choose a conversation first.");
    return *existing;
}

std::string ChatModel::openContact(std::string contactId, std::string name)
{
    require(!contactId.empty() && validText(name, 256), "Choose a personal contact.");
    const auto conversations = mDatabase->conversations();
    const auto existing = std::ranges::find(conversations, contactId, &Conversation::contactId);
    if (existing != conversations.end())
    {
        if (existing->name != name)
        {
            auto updated = *existing;
            updated.name = std::move(name);
            require(mDatabase->upsertConversation(updated), "Could not update the conversation.");
        }
        return existing->id;
    }

    Conversation created;
    created.id = randomId();
    created.contactId = std::move(contactId);
    created.name = std::move(name);
    created.updatedAtMs = nowMs();
    require(mDatabase->upsertConversation(created), "Could not create the conversation.");
    return created.id;
}

Conversation ChatModel::selectIncomingConversation(const std::string& fingerprint, const std::string& targetId) const
{
    const auto conversations = mDatabase->conversations();
    const auto unverifiedId = "peer-" + fingerprint;
    const auto existing = std::ranges::find_if(conversations, [&](const Conversation& item) {
        return item.peerFingerprint == fingerprint || item.id == unverifiedId;
    });
    if (!targetId.empty())
    {
        const auto selected = std::ranges::find(conversations, targetId, &Conversation::id);
        require(selected != conversations.end(), "Choose a conversation first.");
        require(!selected->contactId.empty(), "Choose a personal contact for this invitation.");
        require(existing == conversations.end() || existing->id == targetId,
                "This device already has a conversation. Open it from the chat list.");
        return *selected;
    }
    if (existing != conversations.end())
    {
        return *existing;
    }

    Conversation selected;
    selected.id = unverifiedId;
    selected.name = "Chat device " + fingerprint.substr(0, 8);
    selected.updatedAtMs = nowMs();
    return selected;
}

void ChatModel::persistIncomingConversation(const Conversation& conversation)
{
    require(mDatabase->upsertConversation(conversation), "Could not save the conversation.");
}

void ChatModel::pinPeer(const std::string& id, const std::string& fingerprint)
{
    auto selected = conversation(id);
    const auto conversations = mDatabase->conversations();
    const auto samePeer = std::ranges::find(conversations, fingerprint, &Conversation::peerFingerprint);
    require(samePeer == conversations.end() || samePeer->id == id,
            "This device is already paired in another conversation.");
    selected.peerFingerprint = fingerprint;
    require(mDatabase->upsertConversation(selected), "Could not save the verified device.");
}

void ChatModel::queueOutgoing(const std::string& id, std::string text, std::string replyToMessageId)
{
    require(!conversation(id).peerFingerprint.empty(), "Verify the other device before sending messages.");
    require(validText(text, maxTextBytes), "Enter a message of up to 4 KiB of text.");
    require(replyToMessageId.empty() || mDatabase->hasMessage(id, replyToMessageId),
            "The message to reply to is unavailable in this conversation.");
    require(mDatabase->putMessage({randomId(), id, std::move(text), nowMs(), true, false, std::move(replyToMessageId)}),
            "Could not save the message to the outbox.");
}

bool ChatModel::storeIncoming(const std::string& id, const std::string& messageId,
                              std::string text, std::string replyToMessageId)
{
    return mDatabase->putMessage({messageId, id, std::move(text), nowMs(), false, true, std::move(replyToMessageId)});
}

bool ChatModel::markDelivered(const std::string& id, const std::string& messageId)
{
    return mDatabase->markDelivered(id, messageId);
}

std::optional<Message> ChatModel::nextPending(const std::string& id) const
{
    auto messages = mDatabase->messages(id);
    const auto pending = std::ranges::find_if(messages, [](const Message& message) {
        return message.outgoing && !message.delivered;
    });
    if (pending == messages.end())
    {
        return std::nullopt;
    }
    return std::move(*pending);
}

void ChatModel::createGroup(std::string name)
{
    name = groupName(std::move(name));
    const auto groups = mDatabase->groups();
    require(std::ranges::find(groups, name, &Group::name) == groups.end(), "A group with this name already exists.");
    const auto last = std::ranges::max_element(groups, {}, &Group::position);
    const auto position = last == groups.end() ? 0 : last->position + 1;
    require(mDatabase->upsertGroup({randomId(), std::move(name), position}), "Could not create the group.");
}

void ChatModel::renameGroup(const std::string& id, std::string name)
{
    name = groupName(std::move(name));
    const auto groups = mDatabase->groups();
    const auto group = std::ranges::find(groups, id, &Group::id);
    require(group != groups.end(), "This group no longer exists.");
    const auto duplicate = std::ranges::find(groups, name, &Group::name);
    require(duplicate == groups.end() || duplicate->id == id, "A group with this name already exists.");
    require(mDatabase->upsertGroup({id, std::move(name), group->position}), "Could not rename the group.");
}

void ChatModel::moveConversation(const std::string& id, const std::string& groupId)
{
    static_cast<void>(conversation(id));
    const auto groups = mDatabase->groups();
    require(groupId.empty() || std::ranges::find(groups, groupId, &Group::id) != groups.end(),
            "This group no longer exists.");
    require(mDatabase->moveConversation(id, groupId), "Could not move the conversation.");
}

void ChatModel::deleteGroup(const std::string& id)
{
    require(!id.empty() && mDatabase->deleteGroup(id), "Could not remove the group.");
}

} // namespace ucf::service::chat
