#include "ChatService.h"

#include <utility>

#include <ucf/CoreFramework/ICoreFramework.h>
#include <ucf/services/ChatService/ChatServiceCreator.h>
#include <ucf/services/ClientInfoService/IClientInfoService.h>
#include <ucf/services/DataWarehouseService/DatabaseConfig.h>
#include <ucf/services/DataWarehouseService/IDataWarehouseService.h>

#include "ChatManager.h"

namespace ucf::service {

namespace impl {

std::shared_ptr<IChatService> createChatService(framework::ICoreFrameworkWPtr framework)
{
    return std::make_shared<ChatService>(std::move(framework));
}

} // namespace impl

ChatService::ChatService(framework::ICoreFrameworkWPtr framework)
    : m_framework(std::move(framework))
    , m_manager(std::make_shared<chat::ChatManager>())
{
}

std::string ChatService::getServiceName() const
{
    return "ChatService";
}

std::vector<ServiceDependency> ChatService::dependencies() const
{
    return {{typeid(IClientInfoService), DependencyKind::Required},
            {typeid(IDataWarehouseService), DependencyKind::Required}};
}

void ChatService::initService()
{
    const auto core = m_framework.lock();
    if (!core) return;

    const auto client = core->getService<IClientInfoService>().lock();
    const auto database = core->getService<IDataWarehouseService>().lock();
    if (!client || !database) return;

    const auto dbId = client->getSharedDBConfig().getDBId();
    const auto appData = client->getAppDataStoragePath();
    const auto self = shared_from_this();
    {
        const std::scoped_lock lock(m_lifecycleMutex);
        if (m_stopping) return;

        m_database = database;
        m_dbId = dbId;
        if (!appData.empty())
        {
            m_identityDirectory = std::filesystem::path(std::u8string(appData.begin(), appData.end())) / "chat-identity";
        }
        m_manager->setNotificationSink(self);
        database->registerCallback(self);
    }

    // Register first so there is no gap between checking readiness and listening for it.
    if (database->isDatabaseReady(dbId)) OnDatabaseInitialized(dbId);
    else onChatChanged();
}

void ChatService::deinitService()
{
    std::shared_ptr<IDataWarehouseService> database;
    {
        const std::scoped_lock lock(m_lifecycleMutex);
        m_stopping = true;
        database = m_database.lock();
        m_database.reset();
    }

    if (database) database->unRegisterCallback(shared_from_this());
    m_manager->shutdown();
}

void ChatService::OnDatabaseInitialized(const std::string& dbId)
{
    std::shared_ptr<IDataWarehouseService> database;
    std::filesystem::path identityDirectory;
    {
        const std::scoped_lock lock(m_lifecycleMutex);
        if (m_stopping || dbId != m_dbId) return;
        database = m_database.lock();
        identityDirectory = m_identityDirectory;
    }

    if (database) m_manager->bindDatabase(std::move(database), dbId, std::move(identityDirectory));
}

void ChatService::onChatChanged()
{
    {
        const std::scoped_lock lock(m_lifecycleMutex);
        if (m_stopping) return;
    }
    fireNotification(&IChatServiceCallback::onChatChanged);
}

chat::Snapshot ChatService::snapshot(const std::string& conversationId) const
{
    return m_manager->snapshot(conversationId);
}

std::string ChatService::openContact(std::string contactId, std::string name)
{
    return m_manager->openContact(std::move(contactId), std::move(name));
}

void ChatService::createInvite(std::string conversationId)
{
    m_manager->createInvite(std::move(conversationId));
}

std::string ChatService::acceptInvite(std::string text, std::string conversationId)
{
    return m_manager->acceptInvite(std::move(text), std::move(conversationId));
}

void ChatService::applyAnswer(std::string conversationId, std::string text)
{
    m_manager->applyAnswer(std::move(conversationId), std::move(text));
}

void ChatService::verifyPeer(std::string conversationId)
{
    m_manager->verifyPeer(std::move(conversationId));
}

void ChatService::disconnect(std::string conversationId)
{
    m_manager->disconnect(std::move(conversationId));
}

bool ChatService::sendText(std::string conversationId, std::string text, std::string replyToMessageId)
{
    return m_manager->sendText(std::move(conversationId), std::move(text), std::move(replyToMessageId));
}

void ChatService::createGroup(std::string name)
{
    m_manager->createGroup(std::move(name));
}

void ChatService::renameGroup(std::string groupId, std::string name)
{
    m_manager->renameGroup(std::move(groupId), std::move(name));
}

void ChatService::moveConversation(std::string conversationId, std::string groupId)
{
    m_manager->moveConversation(std::move(conversationId), std::move(groupId));
}

void ChatService::deleteGroup(std::string groupId)
{
    m_manager->deleteGroup(std::move(groupId));
}

} // namespace ucf::service
