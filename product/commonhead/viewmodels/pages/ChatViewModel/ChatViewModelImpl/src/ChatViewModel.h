#pragma once

#include <memory>

#include <commonhead/utilities/VMNotificationHelper/VMNotificationHelper.h>
#include <commonhead/viewmodels/ChatViewModel/IChatViewModel.h>
#include <ucf/services/ChatService/IChatServiceCallback.h>

namespace ucf::service {
class IChatService;
}

namespace commonHead::viewModels {

class ChatViewModel final
    : public IChatViewModel
    , public commonHead::utilities::VMNotificationHelper<IChatViewModelCallback>
    , public ucf::service::IChatServiceCallback
    , public std::enable_shared_from_this<ChatViewModel>
{
public:
    using IChatViewModel::IChatViewModel;

    [[nodiscard]] std::string getViewModelName() const override;
    [[nodiscard]] model::ChatSnapshot snapshot(const std::string& conversationId) const override;
    std::string openContact(std::string contactId, std::string name) override;
    void createInvite(std::string conversationId) override;
    std::string acceptInvite(std::string text, std::string conversationId) override;
    void applyAnswer(std::string conversationId, std::string text) override;
    void verifyPeer(std::string conversationId) override;
    void disconnect(std::string conversationId) override;
    bool sendText(std::string conversationId, std::string text, std::string replyToMessageId) override;
    void createGroup(std::string name) override;
    void renameGroup(std::string groupId, std::string name) override;
    void moveConversation(std::string conversationId, std::string groupId) override;
    void deleteGroup(std::string groupId) override;

protected:
    void init() override;

private:
    void onChatChanged() override;
    [[nodiscard]] std::shared_ptr<ucf::service::IChatService> lockService() const;
};

} // namespace commonHead::viewModels
