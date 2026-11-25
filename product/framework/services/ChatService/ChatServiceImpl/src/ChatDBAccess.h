#pragma once

#include <memory>
#include <string>
#include <vector>

#include <ucf/services/ChatService/ChatTypes.h>

namespace ucf::service {
class IDataWarehouseService;

namespace chat {

class ChatDBAccess
{
public:
    ChatDBAccess(std::shared_ptr<IDataWarehouseService> database, std::string dbId);

    [[nodiscard]] bool ready() const;
    [[nodiscard]] std::vector<Group> groups() const;
    [[nodiscard]] std::vector<Conversation> conversations() const;
    [[nodiscard]] std::vector<DirectoryRelation> directoryRelations() const;
    [[nodiscard]] std::vector<Message> messages(const std::string& conversationId) const;
    [[nodiscard]] bool hasMessage(const std::string& conversationId, const std::string& messageId) const;

    bool upsertGroup(const Group& group);
    bool deleteGroup(const std::string& groupId);
    bool upsertConversation(const Conversation& conversation);
    bool moveConversation(const std::string& conversationId, const std::string& groupId);
    bool putMessage(const Message& message);
    bool markDelivered(const std::string& conversationId, const std::string& messageId);

private:
    std::shared_ptr<IDataWarehouseService> mDatabase;
    std::string mDbId;
};

} // namespace chat
} // namespace ucf::service
