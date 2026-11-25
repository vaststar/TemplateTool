#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ucf::service::chat {

enum class ConnectionState
{
    Disconnected, PreparingInvite, AwaitingAnswer, Connecting, VerifyPeer, Connected, Failed
};

struct Group
{
    std::string id;
    std::string name;
    std::int64_t position = 0;
};

struct Conversation
{
    std::string id;
    std::string contactId;
    std::string name;
    std::string peerFingerprint;
    std::string lastMessage;
    std::int64_t updatedAtMs = 0;
};

struct DirectoryRelation
{
    std::string id;
    std::string parentId;
    std::string childId;
};

struct Message
{
    std::string id;
    std::string conversationId;
    std::string text;
    std::int64_t timestampMs = 0;
    bool outgoing = false;
    bool delivered = false;
    std::string replyToMessageId{};
};

struct Connection
{
    ConnectionState state = ConnectionState::Disconnected;
    std::string invite;
    std::string answer;
    std::string localFingerprint;
    std::string remoteFingerprint;
    std::string error;
};

struct Snapshot
{
    bool ready = false;
    std::vector<Group> groups;
    std::vector<Conversation> conversations;
    std::vector<DirectoryRelation> directoryRelations;
    std::vector<Message> messages;
    Connection connection;
    std::string error;
};

} // namespace ucf::service::chat
