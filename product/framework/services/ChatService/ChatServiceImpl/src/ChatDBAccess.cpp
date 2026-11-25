#include "ChatDBAccess.h"

#include <algorithm>
#include <cstdint>
#include <tuple>
#include <utility>

#include <ucf/services/DataWarehouseSchema/DataWarehouseSchemas.h>
#include <ucf/services/DataWarehouseService/DatabaseDataRecord.h>
#include <ucf/services/DataWarehouseService/DatabaseDataValue.h>
#include <ucf/services/DataWarehouseService/IDataWarehouseService.h>

namespace ucf::service::chat {
namespace {

using GroupTable = db::schema::ChatGroupTable;
using ConversationTable = db::schema::ChatConversationTable;
using RelationTable = db::schema::ChatDirectoryRelationTable;
using MessageTable = db::schema::ChatMessageTable;
using model::DBOperatorType;

constexpr std::int64_t Containment = 0;

const model::DBColumnFields GroupColumns{
    GroupTable::IdField, GroupTable::NameField, GroupTable::PositionField
};

const model::DBColumnFields ConversationColumns{
    ConversationTable::IdField, ConversationTable::ContactIdField, ConversationTable::NameField,
    ConversationTable::PeerFingerprintField, ConversationTable::LastMessageField, ConversationTable::UpdatedAtMsField
};

bool insertMembership(IDataWarehouseService& database, const std::string& dbId,
                      const std::string& conversationId, const std::string& groupId)
{
    // Each conversation has one stable containment ID, including after an ungroup/regroup.
    return database.insertIntoDatabase(dbId, RelationTable::TableName,
        {RelationTable::RelationIdField, RelationTable::ChildIdField,
         RelationTable::ParentIdField, RelationTable::RelationTypeField},
        {{"contains:" + conversationId, conversationId, groupId, Containment}});
}

std::string messageKey(const std::string& conversationId, const std::string& messageId)
{
    return std::to_string(conversationId.size()) + ':' + conversationId + messageId;
}

} // namespace

ChatDBAccess::ChatDBAccess(std::shared_ptr<IDataWarehouseService> database, std::string dbId)
    : mDatabase(std::move(database))
    , mDbId(std::move(dbId))
{
}

bool ChatDBAccess::ready() const
{
    return mDatabase && mDatabase->isDatabaseReady(mDbId);
}

std::vector<Group> ChatDBAccess::groups() const
{
    std::vector<Group> result;
    if (!ready())
    {
        return result;
    }

    mDatabase->fetchFromDatabase(mDbId, GroupTable::TableName, GroupColumns, {},
        [&result](const model::DatabaseDataRecords& rows) {
            result.reserve(rows.size());
            for (const auto& row : rows)
            {
                result.push_back(Group{
                    row.getColumnData(GroupTable::IdField).getStringValue(),
                    row.getColumnData(GroupTable::NameField).getStringValue(),
                    row.getColumnData(GroupTable::PositionField).getIntValue()
                });
            }
        });
    std::ranges::sort(result, [](const Group& left, const Group& right) {
        return std::tie(left.position, left.id) < std::tie(right.position, right.id);
    });
    return result;
}

std::vector<Conversation> ChatDBAccess::conversations() const
{
    std::vector<Conversation> result;
    if (!ready())
    {
        return result;
    }

    mDatabase->fetchFromDatabase(mDbId, ConversationTable::TableName, ConversationColumns, {},
        [&result](const model::DatabaseDataRecords& rows) {
            result.reserve(rows.size());
            for (const auto& row : rows)
            {
                result.push_back(Conversation{
                    row.getColumnData(ConversationTable::IdField).getStringValue(),
                    row.getColumnData(ConversationTable::ContactIdField).getStringValue(),
                    row.getColumnData(ConversationTable::NameField).getStringValue(),
                    row.getColumnData(ConversationTable::PeerFingerprintField).getStringValue(),
                    row.getColumnData(ConversationTable::LastMessageField).getStringValue(),
                    row.getColumnData(ConversationTable::UpdatedAtMsField).getIntValue()
                });
            }
        });
    std::ranges::sort(result, [](const Conversation& left, const Conversation& right) {
        if (left.updatedAtMs != right.updatedAtMs)
        {
            return left.updatedAtMs > right.updatedAtMs;
        }
        return left.id < right.id;
    });
    return result;
}

std::vector<DirectoryRelation> ChatDBAccess::directoryRelations() const
{
    std::vector<DirectoryRelation> result;
    if (!ready())
    {
        return result;
    }

    mDatabase->fetchFromDatabase(mDbId, RelationTable::TableName,
        {RelationTable::RelationIdField, RelationTable::ParentIdField, RelationTable::ChildIdField},
        {{RelationTable::RelationTypeField, Containment, DBOperatorType::Equal}},
        [&result](const model::DatabaseDataRecords& rows) {
            result.reserve(rows.size());
            for (const auto& row : rows)
            {
                result.push_back(DirectoryRelation{
                    row.getColumnData(RelationTable::RelationIdField).getStringValue(),
                    row.getColumnData(RelationTable::ParentIdField).getStringValue(),
                    row.getColumnData(RelationTable::ChildIdField).getStringValue()
                });
            }
        });
    return result;
}

std::vector<Message> ChatDBAccess::messages(const std::string& conversationId) const
{
    if (!ready())
    {
        return {};
    }

    // The warehouse has no ORDER BY support. Sort on SQLite's local insertion sequence.
    using StoredMessage = std::pair<std::int64_t, Message>;
    std::vector<StoredMessage> ordered;
    mDatabase->fetchFromDatabase(mDbId, MessageTable::TableName,
        {MessageTable::SequenceField, MessageTable::IdField, MessageTable::TextField,
         MessageTable::TimestampMsField, MessageTable::OutgoingField, MessageTable::DeliveredField,
         MessageTable::ReplyToMessageIdField},
        {{MessageTable::ConversationIdField, conversationId, DBOperatorType::Equal}},
        [&ordered, &conversationId](const model::DatabaseDataRecords& rows) {
            ordered.reserve(rows.size());
            for (const auto& row : rows)
            {
                ordered.emplace_back(row.getColumnData(MessageTable::SequenceField).getIntValue(), Message{
                    row.getColumnData(MessageTable::IdField).getStringValue(),
                    conversationId,
                    row.getColumnData(MessageTable::TextField).getStringValue(),
                    row.getColumnData(MessageTable::TimestampMsField).getIntValue(),
                    row.getColumnData(MessageTable::OutgoingField).getIntValue() != 0,
                    row.getColumnData(MessageTable::DeliveredField).getIntValue() != 0,
                    row.getColumnData(MessageTable::ReplyToMessageIdField).getStringValue()
                });
            }
        });
    std::ranges::sort(ordered, {}, &StoredMessage::first);

    std::vector<Message> result;
    result.reserve(ordered.size());
    for (auto& row : ordered)
    {
        result.push_back(std::move(row.second));
    }
    return result;
}

bool ChatDBAccess::hasMessage(const std::string& conversationId, const std::string& messageId) const
{
    return ready() && mDatabase->count(mDbId, MessageTable::TableName,
        {{MessageTable::KeyField, messageKey(conversationId, messageId), DBOperatorType::Equal}}) == 1;
}

bool ChatDBAccess::upsertGroup(const Group& group)
{
    return ready() && mDatabase->insertIntoDatabase(mDbId, GroupTable::TableName, GroupColumns,
        {{group.id, group.name, group.position}});
}

bool ChatDBAccess::deleteGroup(const std::string& groupId)
{
    return ready() && mDatabase->atomicWrite(mDbId, [&] {
        return mDatabase->deleteFromDatabase(mDbId, RelationTable::TableName,
                   {{RelationTable::ParentIdField, groupId, DBOperatorType::Equal},
                    {RelationTable::RelationTypeField, Containment, DBOperatorType::Equal}}) >= 0
            && mDatabase->deleteFromDatabase(mDbId, GroupTable::TableName,
                   {{GroupTable::IdField, groupId, DBOperatorType::Equal}}) == 1;
    });
}

bool ChatDBAccess::upsertConversation(const Conversation& conversation)
{
    return ready() && mDatabase->insertIntoDatabase(mDbId, ConversationTable::TableName, ConversationColumns,
        {{conversation.id, conversation.contactId, conversation.name,
          conversation.peerFingerprint, conversation.lastMessage, conversation.updatedAtMs}});
}

bool ChatDBAccess::moveConversation(const std::string& conversationId, const std::string& groupId)
{
    return ready() && mDatabase->atomicWrite(mDbId, [&] {
        if (mDatabase->count(mDbId, ConversationTable::TableName,
                {{ConversationTable::IdField, conversationId, DBOperatorType::Equal}}) != 1)
        {
            return false;
        }

        const model::ListsOfWhereCondition child{
            {RelationTable::ChildIdField, conversationId, DBOperatorType::Equal}
        };
        const model::ListsOfWhereCondition membership{
            {RelationTable::ChildIdField, conversationId, DBOperatorType::Equal},
            {RelationTable::RelationTypeField, Containment, DBOperatorType::Equal}
        };
        if (groupId.empty())
        {
            return mDatabase->deleteFromDatabase(mDbId, RelationTable::TableName, membership) >= 0;
        }
        if (mDatabase->count(mDbId, GroupTable::TableName,
                {{GroupTable::IdField, groupId, DBOperatorType::Equal}}) != 1)
        {
            return false;
        }

        const auto changed = mDatabase->updateInDatabase(mDbId, RelationTable::TableName,
            {RelationTable::ParentIdField}, {groupId}, membership);
        if (changed != 0)
        {
            return changed == 1;
        }
        return mDatabase->count(mDbId, RelationTable::TableName, child) == 0 &&
            insertMembership(*mDatabase, mDbId, conversationId, groupId);
    });
}

bool ChatDBAccess::putMessage(const Message& message)
{
    return ready() && mDatabase->atomicWrite(mDbId, [&] {
        const auto key = messageKey(message.conversationId, message.id);
        const model::ListsOfWhereCondition messageLookup{
            {MessageTable::KeyField, key, DBOperatorType::Equal}
        };

        // The warehouse inserts with REPLACE. Check for an existing message in the same
        // transaction so retransmission cannot overwrite history or change delivery state.
        const auto count = mDatabase->count(mDbId, MessageTable::TableName, messageLookup);
        if (count == 1)
        {
            bool matches = false;
            mDatabase->fetchFromDatabase(mDbId, MessageTable::TableName,
                {MessageTable::TextField, MessageTable::OutgoingField, MessageTable::ReplyToMessageIdField},
                messageLookup,
                [&matches, &message](const model::DatabaseDataRecords& rows) {
                    if (rows.size() != 1)
                    {
                        return;
                    }
                    const auto& original = rows.front();
                    const auto replyTo = original.getColumnData(MessageTable::ReplyToMessageIdField);
                    matches = original.getColumnData(MessageTable::TextField).getStringValue() == message.text &&
                        (original.getColumnData(MessageTable::OutgoingField).getIntValue() != 0) == message.outgoing &&
                        replyTo.holdsType<model::DBSupportedTypes::STRING>() &&
                        replyTo.getStringValue() == message.replyToMessageId;
                });
            return matches;
        }
        if (count != 0)
        {
            return false;
        }

        return mDatabase->insertIntoDatabase(mDbId, MessageTable::TableName,
                   {MessageTable::KeyField, MessageTable::IdField, MessageTable::ConversationIdField,
                    MessageTable::TextField, MessageTable::TimestampMsField, MessageTable::OutgoingField,
                    MessageTable::DeliveredField, MessageTable::ReplyToMessageIdField},
                   {{key, message.id, message.conversationId, message.text, message.timestampMs,
                     message.outgoing, message.delivered, message.replyToMessageId}})
            && mDatabase->updateInDatabase(mDbId, ConversationTable::TableName,
                   {ConversationTable::LastMessageField, ConversationTable::UpdatedAtMsField},
                   {message.text, message.timestampMs},
                   {{ConversationTable::IdField, message.conversationId, DBOperatorType::Equal}}) == 1;
    });
}

bool ChatDBAccess::markDelivered(const std::string& conversationId, const std::string& messageId)
{
    return ready() && mDatabase->updateInDatabase(mDbId, MessageTable::TableName,
        {MessageTable::DeliveredField}, {true},
        {{MessageTable::KeyField, messageKey(conversationId, messageId), DBOperatorType::Equal},
         {MessageTable::OutgoingField, true, DBOperatorType::Equal}}) == 1;
}

} // namespace ucf::service::chat
