#pragma once

#include <string>

#include <ucf/services/ChatService/ChatTypes.h>
#include <ucf/services/ChatService/IChatServiceCallback.h>
#include <ucf/services/ServiceDeclaration/IService.h>
#include <ucf/utilities/NotificationHelper/INotificationHelper.h>

namespace ucf::service {

class IChatService : public IService, public virtual utilities::INotificationHelper<IChatServiceCallback>
{
public:
    virtual chat::Snapshot snapshot(const std::string& conversationId) const = 0;
    virtual std::string openContact(std::string contactId, std::string name) = 0;
    virtual void createInvite(std::string conversationId) = 0;
    virtual std::string acceptInvite(std::string text, std::string conversationId = {}) = 0;
    virtual void applyAnswer(std::string conversationId, std::string text) = 0;
    virtual void verifyPeer(std::string conversationId) = 0;
    virtual void disconnect(std::string conversationId) = 0;
    virtual bool sendText(std::string conversationId, std::string text, std::string replyToMessageId = {}) = 0;
    virtual void createGroup(std::string name) = 0;
    virtual void renameGroup(std::string groupId, std::string name) = 0;
    virtual void moveConversation(std::string conversationId, std::string groupId) = 0;
    virtual void deleteGroup(std::string groupId) = 0;
};

} // namespace ucf::service
