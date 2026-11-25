#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

#include <ucf/services/ChatService/ChatTypes.h>
#include <ucf/utilities/SinkNotifier/SinkNotifier.h>
#include <ucf/utilities/ThreadPoolUtils/ThreadPoolWrapper.h>

#include "ChatNotificationSink.h"
#include "ChatPeer.h"
#include "ChatProtocol.h"

namespace ucf::service {
class IDataWarehouseService;

namespace chat {

class ChatModel;

class ChatManager final : public utilities::SinkNotifier<IChatNotificationSink>,
                          public std::enable_shared_from_this<ChatManager>
{
public:
    ChatManager();
    ~ChatManager() override;

    void bindDatabase(std::shared_ptr<IDataWarehouseService> database, std::string dbId,
                      std::filesystem::path identityDirectory);
    void shutdown();

    [[nodiscard]] Snapshot snapshot(const std::string& conversationId) const;
    std::string openContact(std::string contactId, std::string name);
    void createInvite(std::string conversationId);
    std::string acceptInvite(std::string text, std::string conversationId);
    void applyAnswer(std::string conversationId, std::string text);
    void verifyPeer(std::string conversationId);
    void disconnect(std::string conversationId);
    bool sendText(std::string conversationId, std::string text, std::string replyToMessageId);
    void createGroup(std::string name);
    void renameGroup(std::string groupId, std::string name);
    void moveConversation(std::string conversationId, std::string groupId);
    void deleteGroup(std::string groupId);

private:
    struct Session
    {
        std::uint64_t generation = 0;
        Invitation localDescription;
        Connection connection;
        std::unique_ptr<ChatPeer> peer;
        std::string inFlight;
        bool open = false;
        bool localReady = false;
        bool remoteReady = false;
    };

    using PeerEvent = void (ChatManager::*)(const std::string&, Session&, std::string);

    void changed();
    void perform(const std::function<void()>& command);
    const ChatIdentity& identity();
    std::function<void(std::string)> peerEvent(const std::string& id, std::uint64_t generation, PeerEvent event);
    Session& makeSession(const std::string& id, Invitation invite, std::string remoteFingerprint);
    static void fail(Session& session, std::string reason);
    void onLocalDescription(const std::string&, Session& session, std::string sdp);
    void onOpen(const std::string& id, Session& session, std::string fingerprint);
    void confirm(const std::string& id, Session& session);
    void establish(const std::string& id, Session& session);
    void sendPending(const std::string& id, Session& session);
    void onFrame(const std::string& id, Session& session, std::string bytes);
    void onClosed(const std::string&, Session& session, std::string reason);

    mutable std::mutex m_mutex;
    std::unique_ptr<ChatModel> m_model;
    std::filesystem::path m_identityDirectory;
    std::optional<ChatIdentity> m_identity;
    std::map<std::string, Session> m_sessions;
    std::string m_error;
    std::uint64_t m_generation = 0;
    bool m_stopping = false;
    utilities::ThreadPoolWrapper m_events{1, "ChatEvents"};
};

} // namespace chat
} // namespace ucf::service
