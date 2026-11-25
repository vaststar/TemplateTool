#include "ChatManager.h"

#include <stdexcept>
#include <utility>

#include "ChatModel.h"
#include "ChatUtils.h"

namespace ucf::service::chat {
namespace {

void require(bool accepted, const char* error)
{
    if (!accepted)
    {
        throw std::runtime_error(error);
    }
}

} // namespace

ChatManager::ChatManager() = default;

ChatManager::~ChatManager()
{
    shutdown();
}

void ChatManager::bindDatabase(std::shared_ptr<IDataWarehouseService> database, std::string dbId,
                               std::filesystem::path identityDirectory)
{
    {
        const std::scoped_lock lock(m_mutex);
        if (m_stopping || !database || m_model)
        {
            return;
        }
        m_model = std::make_unique<ChatModel>(std::move(database), std::move(dbId));
        m_identityDirectory = std::move(identityDirectory);
    }
    changed();
}

void ChatManager::shutdown()
{
    std::map<std::string, Session> sessions;
    {
        const std::scoped_lock lock(m_mutex);
        if (m_stopping)
        {
            return;
        }
        m_stopping = true;
        setNotificationSink({});
        sessions.swap(m_sessions);
        m_model.reset();
    }
    sessions.clear();
    m_events.shutdown();
}

Snapshot ChatManager::snapshot(const std::string& conversationId) const
{
    const std::scoped_lock lock(m_mutex);
    if (m_stopping)
    {
        return {};
    }
    auto result = m_model ? m_model->snapshot(conversationId) : Snapshot{};
    if (!m_error.empty())
    {
        result.error = m_error;
    }
    if (const auto it = m_sessions.find(conversationId); it != m_sessions.end())
    {
        result.connection = it->second.connection;
    }
    return result;
}

std::string ChatManager::openContact(std::string contactId, std::string name)
{
    std::string id;
    perform([&] { id = m_model->openContact(std::move(contactId), std::move(name)); });
    return id;
}

void ChatManager::createInvite(std::string id)
{
    perform([&] {
        static_cast<void>(m_model->conversation(id));
        auto& session = makeSession(id, {false, randomId(), nowMs() + inviteLifetimeMs, {}, {}}, {});
        try
        {
            session.peer->startOffer();
        }
        catch (const std::exception&)
        {
            fail(session, "Could not prepare a direct connection.");
        }
    });
}

std::string ChatManager::acceptInvite(std::string text, std::string targetId)
{
    std::string id;
    perform([&] {
        auto offer = decodeInvitation(text, false);
        require(offer.fingerprint != identity().fingerprint, "This invitation belongs to this device.");
        for (const auto& [knownId, session] : m_sessions)
        {
            if (session.localDescription.answer && session.localDescription.id == offer.id &&
                session.connection.remoteFingerprint == offer.fingerprint && session.peer &&
                (targetId.empty() || targetId == knownId))
            {
                id = knownId;
                return;
            }
        }

        const auto selected = m_model->selectIncomingConversation(offer.fingerprint, targetId);
        auto& session = makeSession(selected.id, {true, offer.id, offer.expiresAtMs, {}, {}}, offer.fingerprint);
        try
        {
            session.peer->setRemoteDescription(offer.sdp, true);
            m_model->persistIncomingConversation(selected);
        }
        catch (const std::exception&)
        {
            m_sessions.erase(selected.id);
            throw std::runtime_error("Could not accept this chat invitation.");
        }
        id = selected.id;
    });
    return id;
}

void ChatManager::applyAnswer(std::string id, std::string text)
{
    perform([&] {
        auto answer = decodeInvitation(text, true);
        auto it = m_sessions.find(id);
        require(it != m_sessions.end() && it->second.connection.state == ConnectionState::AwaitingAnswer,
                "Create an invitation before pasting an answer.");
        auto& session = it->second;
        require(answer.id == session.localDescription.id && answer.expiresAtMs == session.localDescription.expiresAtMs,
                "This answer does not match the current invitation.");
        require(answer.fingerprint != identity().fingerprint, "This answer belongs to this device.");
        session.connection.remoteFingerprint = answer.fingerprint;
        session.connection.state = ConnectionState::Connecting;
        try
        {
            session.peer->setRemoteDescription(answer.sdp, false);
        }
        catch (const std::exception&)
        {
            fail(session, "Could not apply this answer. Create a new invitation.");
        }
    });
}

void ChatManager::verifyPeer(std::string id)
{
    perform([&] {
        auto it = m_sessions.find(id);
        require(it != m_sessions.end() && it->second.open && it->second.connection.state == ConnectionState::VerifyPeer,
                "Connect to the other device before confirming its fingerprint.");
        auto& session = it->second;
        m_model->pinPeer(id, session.connection.remoteFingerprint);
        confirm(id, session);
    });
}

void ChatManager::disconnect(std::string id)
{
    perform([&] { m_sessions.erase(id); });
}

bool ChatManager::sendText(std::string id, std::string text, std::string replyToMessageId)
{
    bool queued = false;
    perform([&] {
        m_model->queueOutgoing(id, std::move(text), std::move(replyToMessageId));
        queued = true;
        if (auto it = m_sessions.find(id); it != m_sessions.end())
        {
            sendPending(id, it->second);
        }
    });
    return queued;
}

void ChatManager::createGroup(std::string name)
{
    perform([&] { m_model->createGroup(std::move(name)); });
}

void ChatManager::renameGroup(std::string id, std::string name)
{
    perform([&] { m_model->renameGroup(id, std::move(name)); });
}

void ChatManager::moveConversation(std::string id, std::string groupId)
{
    perform([&] { m_model->moveConversation(id, groupId); });
}

void ChatManager::deleteGroup(std::string id)
{
    perform([&] { m_model->deleteGroup(id); });
}

void ChatManager::changed()
{
    notifySink(&IChatNotificationSink::onChatChanged);
}

void ChatManager::perform(const std::function<void()>& command)
{
    {
        const std::scoped_lock lock(m_mutex);
        if (m_stopping)
        {
            return;
        }
        m_error.clear();
        try
        {
            require(m_model && m_model->ready(), "Chat history is not available yet.");
            command();
        }
        catch (const std::exception& exception)
        {
            m_error = exception.what();
        }
    }
    changed();
}

const ChatIdentity& ChatManager::identity()
{
    require(!m_identityDirectory.empty(), "Chat identity storage is not available.");
    if (!m_identity)
    {
        m_identity = loadOrCreateChatIdentity(m_identityDirectory);
    }
    return *m_identity;
}

std::function<void(std::string)> ChatManager::peerEvent(const std::string& id, std::uint64_t generation, PeerEvent event)
{
    return [weak = weak_from_this(), id, generation, event](std::string value) {
        if (const auto owner = weak.lock())
        {
            // ChatPeer calls with its callback mutex held. Schedule all state changes on the worker.
            owner->m_events.submit([weak, id, generation, event, value = std::move(value)]() mutable {
                const auto manager = weak.lock();
                if (!manager)
                {
                    return;
                }
                {
                    const std::scoped_lock lock(manager->m_mutex);
                    const auto it = manager->m_sessions.find(id);
                    if (manager->m_stopping || it == manager->m_sessions.end() ||
                        it->second.generation != generation || !it->second.peer)
                    {
                        return;
                    }
                    try
                    {
                        ((*manager).*event)(id, it->second, std::move(value));
                    }
                    catch (const std::exception&)
                    {
                        fail(it->second, "The chat connection could not continue.");
                    }
                }
                manager->changed();
            });
        }
    };
}

ChatManager::Session& ChatManager::makeSession(const std::string& id, Invitation invite, std::string remoteFingerprint)
{
    const auto& localIdentity = identity();
    Session session;
    session.generation = ++m_generation;
    session.localDescription = std::move(invite);
    session.localDescription.fingerprint = localIdentity.fingerprint;
    session.connection.localFingerprint = localIdentity.fingerprint;
    session.connection.remoteFingerprint = std::move(remoteFingerprint);
    session.connection.state = ConnectionState::PreparingInvite;
    session.peer = std::make_unique<ChatPeer>(localIdentity, ChatPeer::Events{
        peerEvent(id, session.generation, &ChatManager::onLocalDescription),
        peerEvent(id, session.generation, &ChatManager::onOpen),
        peerEvent(id, session.generation, &ChatManager::onFrame),
        peerEvent(id, session.generation, &ChatManager::onClosed)});
    return m_sessions.insert_or_assign(id, std::move(session)).first->second;
}

void ChatManager::fail(Session& session, std::string reason)
{
    session.connection.invite.clear();
    session.connection.answer.clear();
    session.connection.error = std::move(reason);
    session.connection.state = ConnectionState::Failed;
    session.open = false;
    session.peer.reset();
}

void ChatManager::onLocalDescription(const std::string&, Session& session, std::string sdp)
{
    if (sdp.find("\na=candidate:") == std::string::npos)
    {
        fail(session, "No local network address is available for this conversation.");
        return;
    }
    session.localDescription.sdp = std::move(sdp);
    auto encoded = encodeInvitation(session.localDescription);
    session.localDescription.sdp.clear();
    if (session.localDescription.answer)
    {
        session.connection.answer = std::move(encoded);
        if (!session.open)
        {
            session.connection.state = ConnectionState::Connecting;
        }
    }
    else
    {
        session.connection.invite = std::move(encoded);
        session.connection.state = ConnectionState::AwaitingAnswer;
    }
}

void ChatManager::onOpen(const std::string& id, Session& session, std::string fingerprint)
{
    if (fingerprint.empty() || fingerprint != session.connection.remoteFingerprint)
    {
        fail(session, "The connected device fingerprint does not match the invitation.");
        return;
    }
    const auto selected = m_model->conversation(id);
    session.open = true;
    session.connection.invite.clear();
    session.connection.answer.clear();
    if (selected.peerFingerprint == fingerprint)
    {
        confirm(id, session);
    }
    else
    {
        session.connection.state = ConnectionState::VerifyPeer;
        if (!selected.peerFingerprint.empty())
        {
            session.connection.error = "The device fingerprint changed. Verify the new device before sending messages.";
        }
    }
}

void ChatManager::confirm(const std::string& id, Session& session)
{
    session.localReady = session.peer->send(encodeFrame({FrameType::Ready, {}, {}}));
    if (!session.localReady)
    {
        fail(session, "The connection closed before the device could be verified.");
        return;
    }
    establish(id, session);
}

void ChatManager::establish(const std::string& id, Session& session)
{
    if (!session.localReady)
    {
        return;
    }
    session.connection.state = session.remoteReady ? ConnectionState::Connected : ConnectionState::Connecting;
    session.connection.error = session.remoteReady ? "" : "Waiting for the other device to finish verification.";
    sendPending(id, session);
}

void ChatManager::sendPending(const std::string& id, Session& session)
{
    if (session.connection.state != ConnectionState::Connected || !session.inFlight.empty())
    {
        return;
    }
    const auto pending = m_model->nextPending(id);
    if (!pending)
    {
        return;
    }
    if (session.peer->send(encodeFrame({FrameType::Text, pending->id, pending->text, pending->replyToMessageId})))
    {
        session.inFlight = pending->id;
    }
    else
    {
        fail(session, "The connection closed. Pending messages are saved locally.");
    }
}

void ChatManager::onFrame(const std::string& id, Session& session, std::string bytes)
{
    auto frame = decodeFrame(bytes);
    if (frame.type == FrameType::Ready)
    {
        session.remoteReady = true;
        establish(id, session);
        return;
    }
    if (session.connection.state != ConnectionState::Connected)
    {
        return;
    }
    if (frame.type == FrameType::Text)
    {
        if (!m_model->storeIncoming(id, frame.id, std::move(frame.text), std::move(frame.replyToMessageId)))
        {
            fail(session, "Could not save the incoming message. Reconnect to retry delivery.");
            return;
        }
        if (!session.peer->send(encodeFrame({FrameType::Receipt, frame.id, {}})))
        {
            fail(session, "The connection closed before the delivery receipt could be sent.");
        }
    }
    else if (session.inFlight == frame.id)
    {
        if (!m_model->markDelivered(id, frame.id))
        {
            fail(session, "Could not save the delivery receipt. Reconnect to retry delivery.");
            return;
        }
        session.inFlight.clear();
        sendPending(id, session);
    }
}

void ChatManager::onClosed(const std::string&, Session& session, std::string reason)
{
    if (reason.empty())
    {
        session.connection = {};
        session.open = false;
        session.peer.reset();
    }
    else
    {
        fail(session, std::move(reason));
    }
}

} // namespace ucf::service::chat
