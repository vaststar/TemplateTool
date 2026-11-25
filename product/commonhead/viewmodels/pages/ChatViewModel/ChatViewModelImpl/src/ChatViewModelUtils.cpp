#include "ChatViewModelUtils.h"

#include <algorithm>
#include <unordered_map>
#include <utility>

namespace commonHead::viewModels::utils {
namespace {

namespace serviceChat = ucf::service::chat;

model::ChatConnectionState connectionState(serviceChat::ConnectionState state)
{
    switch (state)
    {
    case serviceChat::ConnectionState::Disconnected:    return model::ChatConnectionState::Disconnected;
    case serviceChat::ConnectionState::PreparingInvite: return model::ChatConnectionState::PreparingInvite;
    case serviceChat::ConnectionState::AwaitingAnswer:  return model::ChatConnectionState::AwaitingAnswer;
    case serviceChat::ConnectionState::Connecting:      return model::ChatConnectionState::Connecting;
    case serviceChat::ConnectionState::VerifyPeer:      return model::ChatConnectionState::VerifyPeer;
    case serviceChat::ConnectionState::Connected:       return model::ChatConnectionState::Connected;
    case serviceChat::ConnectionState::Failed:          return model::ChatConnectionState::Failed;
    }
    return model::ChatConnectionState::Disconnected;
}

model::ChatDirectory makeDirectory(serviceChat::Snapshot& source)
{
    std::ranges::stable_sort(source.groups, {}, &serviceChat::Group::position);
    std::ranges::stable_sort(source.conversations, std::ranges::greater{},
                             &serviceChat::Conversation::updatedAtMs);

    model::ChatDirectory directory;
    directory.groups.reserve(source.groups.size());
    if (!source.conversations.empty())
    {
        directory.mostRecentConversationId = source.conversations.front().id;
    }

    std::unordered_map<std::string, std::size_t> sectionByGroup;
    for (auto& group : source.groups)
    {
        sectionByGroup.emplace(group.id, directory.groups.size());
        directory.groups.push_back({{std::move(group.id), std::move(group.name), group.position}, {}});
    }

    struct Membership
    {
        std::size_t sectionIndex;
        const serviceChat::DirectoryRelation& relation;
    };
    std::unordered_map<std::string, Membership> membershipByConversation;
    for (const auto& relation : source.directoryRelations)
    {
        if (const auto parent = sectionByGroup.find(relation.parentId); parent != sectionByGroup.end())
        {
            membershipByConversation.try_emplace(relation.childId, Membership{parent->second, relation});
        }
    }

    for (auto& item : source.conversations)
    {
        const auto membership = membershipByConversation.find(item.id);
        model::ChatConversation conversation{
            std::move(item.id), std::move(item.contactId), std::move(item.name), std::move(item.peerFingerprint),
            std::move(item.lastMessage), item.updatedAtMs
        };
        if (membership == membershipByConversation.end())
        {
            directory.ungrouped.push_back(std::move(conversation));
            continue;
        }

        const auto& [sectionIndex, relation] = membership->second;
        directory.groups[sectionIndex].conversations.push_back(std::move(conversation));
        directory.relations.push_back({relation.id, relation.parentId, relation.childId});
    }
    return directory;
}

std::vector<model::ChatMessage> makeMessages(std::vector<serviceChat::Message>& source)
{
    std::vector<model::ChatMessage> messages;
    messages.reserve(source.size());
    std::unordered_map<std::string, std::size_t> messageIndex;
    for (auto& message : source)
    {
        messageIndex.emplace(message.id, messages.size());
        messages.push_back({
            .id = std::move(message.id),
            .conversationId = std::move(message.conversationId),
            .text = std::move(message.text),
            .timestampMs = message.timestampMs,
            .outgoing = message.outgoing,
            .delivered = message.delivered,
            .replyToMessageId = std::move(message.replyToMessageId),
            .replyPreview = std::nullopt
        });
    }
    for (auto& message : messages)
    {
        if (message.replyToMessageId.empty())
        {
            continue;
        }
        if (const auto target = messageIndex.find(message.replyToMessageId); target != messageIndex.end())
        {
            const auto& original = messages[target->second];
            if (original.conversationId == message.conversationId)
            {
                message.replyPreview = model::ChatReplyPreview{original.text, original.outgoing};
            }
        }
    }
    return messages;
}

} // namespace

model::ChatSnapshot toViewModelSnapshot(serviceChat::Snapshot source)
{
    return {
        .ready = source.ready,
        .directory = makeDirectory(source),
        .messages = makeMessages(source.messages),
        .connection = {
            connectionState(source.connection.state),
            std::move(source.connection.invite),
            std::move(source.connection.answer),
            std::move(source.connection.localFingerprint),
            std::move(source.connection.remoteFingerprint),
            std::move(source.connection.error)
        },
        .error = std::move(source.error)
    };
}

} // namespace commonHead::viewModels::utils
