#pragma once

#include <memory>
#include <optional>
#include <string>

#include <ucf/services/ChatService/ChatTypes.h>

namespace ucf::service {
class IDataWarehouseService;

namespace chat {

class ChatDBAccess;

// Persistent chat operations. The manager serializes calls and owns live sessions.
class ChatModel
{
public:
    ChatModel(std::shared_ptr<IDataWarehouseService> database, std::string dbId);
    ~ChatModel();

    [[nodiscard]] bool ready() const;
    [[nodiscard]] Snapshot snapshot(const std::string& id) const;
    [[nodiscard]] Conversation conversation(const std::string& id) const;
    std::string openContact(std::string contactId, std::string name);
    [[nodiscard]] Conversation selectIncomingConversation(const std::string& fingerprint, const std::string& targetId) const;
    void persistIncomingConversation(const Conversation& conversation);
    void pinPeer(const std::string& id, const std::string& fingerprint);

    void queueOutgoing(const std::string& id, std::string text, std::string replyToMessageId);
    [[nodiscard]] bool storeIncoming(const std::string& id, const std::string& messageId,
                                     std::string text, std::string replyToMessageId);
    [[nodiscard]] bool markDelivered(const std::string& id, const std::string& messageId);
    [[nodiscard]] std::optional<Message> nextPending(const std::string& id) const;

    void createGroup(std::string name);
    void renameGroup(const std::string& id, std::string name);
    void moveConversation(const std::string& id, const std::string& groupId);
    void deleteGroup(const std::string& id);

private:
    std::unique_ptr<ChatDBAccess> mDatabase;
};

} // namespace chat
} // namespace ucf::service
