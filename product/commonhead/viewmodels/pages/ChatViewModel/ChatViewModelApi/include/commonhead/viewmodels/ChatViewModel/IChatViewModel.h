#pragma once

#include <string>

#include <commonhead/utilities/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonhead/viewmodels/ChatViewModel/ChatViewModelTypes.h>
#include <commonhead/viewmodels/ViewModelCore/IViewModel.h>

namespace commonHead::viewModels {

class IChatViewModelCallback
{
public:
    virtual ~IChatViewModelCallback() = default;
    virtual void onChatChanged() = 0;
};

class IChatViewModel
    : public IViewModel
    , public virtual commonHead::utilities::IVMNotificationHelper<IChatViewModelCallback>
{
public:
    using IViewModel::IViewModel;

    [[nodiscard]] virtual model::ChatSnapshot snapshot(const std::string& conversationId) const = 0;
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

} // namespace commonHead::viewModels
