#include "ChatViewModel.h"
#include "ChatViewModelUtils.h"

#include <utility>

#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonhead/ServiceLocator/IServiceLocator.h>
#include <commonhead/viewmodels/ChatViewModel/ChatViewModelCreator.h>
#include <ucf/services/ChatService/IChatService.h>

namespace commonHead::viewModels {

namespace impl {

std::shared_ptr<IChatViewModel> createChatViewModel(commonHead::ICommonHeadFrameworkWptr framework)
{
    return std::make_shared<ChatViewModel>(std::move(framework));
}

} // namespace impl

std::string ChatViewModel::getViewModelName() const
{
    return "ChatViewModel";
}

void ChatViewModel::init()
{
    if (const auto service = lockService())
    {
        service->registerCallback(shared_from_this());
    }
}

void ChatViewModel::onChatChanged()
{
    fireNotification(&IChatViewModelCallback::onChatChanged);
}

std::shared_ptr<ucf::service::IChatService> ChatViewModel::lockService() const
{
    if (const auto framework = getCommonHeadFramework().lock())
    {
        if (const auto locator = framework->getServiceLocator())
        {
            return locator->getChatService().lock();
        }
    }
    return {};
}

model::ChatSnapshot ChatViewModel::snapshot(const std::string& conversationId) const
{
    if (const auto service = lockService())
    {
        return utils::toViewModelSnapshot(service->snapshot(conversationId));
    }
    model::ChatSnapshot result;
    result.error = "Chat service is unavailable.";
    return result;
}

std::string ChatViewModel::openContact(std::string contactId, std::string name)
{
    if (const auto service = lockService())
    {
        return service->openContact(std::move(contactId), std::move(name));
    }
    return {};
}

void ChatViewModel::createInvite(std::string conversationId)
{
    if (const auto service = lockService())
    {
        service->createInvite(std::move(conversationId));
    }
}

std::string ChatViewModel::acceptInvite(std::string text, std::string conversationId)
{
    if (const auto service = lockService())
    {
        return service->acceptInvite(std::move(text), std::move(conversationId));
    }
    return {};
}

void ChatViewModel::applyAnswer(std::string conversationId, std::string text)
{
    if (const auto service = lockService())
    {
        service->applyAnswer(std::move(conversationId), std::move(text));
    }
}

void ChatViewModel::verifyPeer(std::string conversationId)
{
    if (const auto service = lockService())
    {
        service->verifyPeer(std::move(conversationId));
    }
}

void ChatViewModel::disconnect(std::string conversationId)
{
    if (const auto service = lockService())
    {
        service->disconnect(std::move(conversationId));
    }
}

bool ChatViewModel::sendText(std::string conversationId, std::string text, std::string replyToMessageId)
{
    if (const auto service = lockService())
    {
        return service->sendText(std::move(conversationId), std::move(text), std::move(replyToMessageId));
    }
    return false;
}

void ChatViewModel::createGroup(std::string name)
{
    if (const auto service = lockService())
    {
        service->createGroup(std::move(name));
    }
}

void ChatViewModel::renameGroup(std::string groupId, std::string name)
{
    if (const auto service = lockService())
    {
        service->renameGroup(std::move(groupId), std::move(name));
    }
}

void ChatViewModel::moveConversation(std::string conversationId, std::string groupId)
{
    if (const auto service = lockService())
    {
        service->moveConversation(std::move(conversationId), std::move(groupId));
    }
}

void ChatViewModel::deleteGroup(std::string groupId)
{
    if (const auto service = lockService())
    {
        service->deleteGroup(std::move(groupId));
    }
}

} // namespace commonHead::viewModels
