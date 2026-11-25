#pragma once

#include <filesystem>
#include <memory>
#include <mutex>

#include <ucf/services/ChatService/IChatService.h>
#include <ucf/services/DataWarehouseService/IDataWarehouseServiceCallback.h>
#include <ucf/utilities/NotificationHelper/NotificationHelper.h>

#include "ChatNotificationSink.h"

namespace ucf::framework { class ICoreFramework; }

namespace ucf::service {

class IDataWarehouseService;
namespace chat { class ChatManager; }

class ChatService final : public IChatService,
                          public utilities::NotificationHelper<IChatServiceCallback>,
                          public IDataWarehouseServiceCallback,
                          public chat::IChatNotificationSink,
                          public std::enable_shared_from_this<ChatService>
{
public:
    explicit ChatService(std::weak_ptr<framework::ICoreFramework> framework);

    std::string getServiceName() const override;
    std::vector<ServiceDependency> dependencies() const override;

    chat::Snapshot snapshot(const std::string& conversationId) const override;
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

    void OnDatabaseInitialized(const std::string& dbId) override;

protected:
    void initService() override;
    void deinitService() override;

private:
    void onChatChanged() override;

    const std::weak_ptr<framework::ICoreFramework> m_framework;
    const std::shared_ptr<chat::ChatManager> m_manager;
    std::mutex m_lifecycleMutex;
    std::weak_ptr<IDataWarehouseService> m_database;
    std::string m_dbId;
    std::filesystem::path m_identityDirectory;
    bool m_stopping = false;
};

} // namespace ucf::service
