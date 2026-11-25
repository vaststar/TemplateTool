#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace commonHead::viewModels::model {

enum class ChatConnectionState
{
    Disconnected, PreparingInvite, AwaitingAnswer, Connecting, VerifyPeer, Connected, Failed
};

struct ChatGroup
{
    std::string id;
    std::string name;
    std::int64_t position = 0;
};

struct ChatConversation
{
    std::string id;
    std::string contactId;
    std::string name;
    std::string peerFingerprint;
    std::string lastMessage;
    std::int64_t updatedAtMs = 0;
};

struct ChatDirectoryRelation
{
    std::string id;
    std::string parentId;
    std::string childId;
};

struct ChatGroupSection
{
    ChatGroup group;
    std::vector<ChatConversation> conversations;
};

struct ChatDirectory
{
    std::vector<ChatGroupSection> groups;
    std::vector<ChatConversation> ungrouped;
    std::vector<ChatDirectoryRelation> relations;
    std::string mostRecentConversationId;
};

struct ChatReplyPreview
{
    std::string text;
    bool outgoing = false;
};

struct ChatMessage
{
    std::string id;
    std::string conversationId;
    std::string text;
    std::int64_t timestampMs = 0;
    bool outgoing = false;
    bool delivered = false;
    std::string replyToMessageId{};
    std::optional<ChatReplyPreview> replyPreview;
};

struct ChatConnection
{
    ChatConnectionState state = ChatConnectionState::Disconnected;
    std::string invite;
    std::string answer;
    std::string localFingerprint;
    std::string remoteFingerprint;
    std::string error;
};

struct ChatSnapshot
{
    bool ready = false;
    ChatDirectory directory;
    std::vector<ChatMessage> messages;
    ChatConnection connection;
    std::string error;
};

} // namespace commonHead::viewModels::model
