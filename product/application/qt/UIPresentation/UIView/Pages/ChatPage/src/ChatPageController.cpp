#include "ChatPage/ChatPageController.h"

#include <utility>
#include <vector>

#include <QClipboard>
#include <QDateTime>
#include <QGuiApplication>
#include <QHash>
#include <QLocale>

#include <commonhead/viewmodels/ChatViewModel/IChatViewModel.h>
#include <commonhead/viewmodels/ViewModelFactory/IViewModelFactory.h>
#include <UIViewModelSignalBridge/emitters/ChatViewModelEmitter.h>

namespace {

namespace model = commonHead::viewModels::model;

QString displayError(const std::string& error)
{
    if (error == "The message to reply to is unavailable in this conversation.")
    {
        return ChatPageController::tr("The message to reply to is unavailable in this conversation.");
    }
    return QString::fromStdString(error);
}

ChatPageController::ConnectionState toUiState(model::ChatConnectionState state)
{
    switch (state)
    {
    case model::ChatConnectionState::Disconnected:    return ChatPageController::Disconnected;
    case model::ChatConnectionState::PreparingInvite: return ChatPageController::PreparingInvite;
    case model::ChatConnectionState::AwaitingAnswer:   return ChatPageController::AwaitingAnswer;
    case model::ChatConnectionState::Connecting:      return ChatPageController::Connecting;
    case model::ChatConnectionState::VerifyPeer:      return ChatPageController::VerifyPeer;
    case model::ChatConnectionState::Connected:       return ChatPageController::Connected;
    case model::ChatConnectionState::Failed:          return ChatPageController::Failed;
    }
    return ChatPageController::Disconnected;
}

bool copyCode(const QString& code)
{
    auto* clipboard = QGuiApplication::clipboard();
    if (!clipboard || code.isEmpty())
    {
        return false;
    }
    clipboard->setText(code);
    return true;
}

QString activityLabel(qint64 timestamp)
{
    if (timestamp <= 0)
    {
        return {};
    }
    const auto time = QDateTime::fromMSecsSinceEpoch(timestamp);
    const auto today = QDate::currentDate();
    if (time.date() == today)
    {
        return time.time().toString(QStringLiteral("HH:mm"));
    }
    if (time.date() == today.addDays(-1))
    {
        return ChatPageController::tr("Yesterday");
    }
    return QLocale().toString(time.date(), QLocale::ShortFormat);
}

QVariantMap conversationRow(const model::ChatConversation& conversation, const QString& id)
{
    return {
        {QStringLiteral("id"), id},
        {QStringLiteral("name"), QString::fromStdString(conversation.name)},
        {QStringLiteral("isContact"), !conversation.contactId.empty()},
        {QStringLiteral("isVerified"), !conversation.peerFingerprint.empty()},
        {QStringLiteral("preview"), QString::fromStdString(conversation.lastMessage)},
        {QStringLiteral("time"), activityLabel(conversation.updatedAtMs)},
        {QStringLiteral("isGroup"), false}
    };
}

} // namespace

ChatPageController::ChatPageController(QObject* parent)
    : UIViewController(parent)
{
}

ChatPageController::~ChatPageController() = default;

void ChatPageController::init()
{
    if (const auto factory = getViewModelFactory())
    {
        m_viewModel = factory->createChatViewModelInstance();
    }
    if (!m_viewModel)
    {
        m_error = tr("Chat is unavailable.");
        emit statusChanged();
        return;
    }

    m_emitter = std::make_shared<UIViewModelSignalBridge::ChatViewModelEmitter>();
    connect(m_emitter.get(), &UIViewModelSignalBridge::ChatViewModelEmitter::chatChanged,
            this, &ChatPageController::refresh, Qt::QueuedConnection);
    m_viewModel->registerCallback(m_emitter);
    m_viewModel->initViewModel();
    refresh();
}

void ChatPageController::onLanguageChanged()
{
    refresh();
}

void ChatPageController::openContact(const QString& contactId, const QString& displayName)
{
    if (!contactId.isEmpty())
    {
        m_pendingContactId = contactId;
        m_pendingContactName = displayName;
        refresh();
    }
}

void ChatPageController::selectConversation(const QString& conversationId)
{
    if (m_selectedId != conversationId)
    {
        m_selectedId = conversationId;
        refresh();
    }
}

void ChatPageController::toggleGroup(const QString& groupId)
{
    if (!m_collapsedGroups.remove(groupId))
    {
        m_collapsedGroups.insert(groupId);
    }
    refresh();
}

bool ChatPageController::createGroup(const QString& name)
{
    if (!m_ready)
    {
        return false;
    }
    m_viewModel->createGroup(name.trimmed().toStdString());
    refresh();
    return m_error.isEmpty();
}

bool ChatPageController::renameGroup(const QString& groupId, const QString& name)
{
    if (!m_ready)
    {
        return false;
    }
    m_viewModel->renameGroup(groupId.toStdString(), name.trimmed().toStdString());
    refresh();
    return m_error.isEmpty();
}

void ChatPageController::moveConversation(const QString& groupId)
{
    m_viewModel->moveConversation(m_selectedId.toStdString(), groupId.toStdString());
    m_revealSelected = true;
    refresh();
}

void ChatPageController::deleteGroup(const QString& groupId)
{
    m_viewModel->deleteGroup(groupId.toStdString());
    m_collapsedGroups.remove(groupId);
    m_revealSelected = true;
    refresh();
}

void ChatPageController::createInvite()
{
    m_viewModel->createInvite(m_selectedId.toStdString());
    refresh();
}

bool ChatPageController::acceptInvite(const QString& text, bool linkSelectedContact)
{
    if (!m_ready)
    {
        return false;
    }
    const auto selectedContact = linkSelectedContact ? m_selectedId.toStdString() : std::string{};
    const auto conversationId = m_viewModel->acceptInvite(text.trimmed().toStdString(), selectedContact);
    if (!conversationId.empty())
    {
        m_selectedId = QString::fromStdString(conversationId);
        m_revealSelected = true;
    }
    refresh();
    return !conversationId.empty();
}

bool ChatPageController::applyAnswer(const QString& text)
{
    m_viewModel->applyAnswer(m_selectedId.toStdString(), text.trimmed().toStdString());
    refresh();
    return m_connectionState != AwaitingAnswer;
}

bool ChatPageController::copyInvite() const
{
    return copyCode(m_invite);
}

bool ChatPageController::copyAnswer() const
{
    return copyCode(m_answer);
}

void ChatPageController::verifyPeer()
{
    m_viewModel->verifyPeer(m_selectedId.toStdString());
    refresh();
}

void ChatPageController::disconnectPeer()
{
    m_viewModel->disconnect(m_selectedId.toStdString());
    refresh();
}

bool ChatPageController::sendText(const QString& text, const QString& replyToMessageId)
{
    if (!m_ready || !m_conversation.value(QStringLiteral("isVerified")).toBool() || text.trimmed().isEmpty())
    {
        return false;
    }
    const bool queued = m_viewModel->sendText(m_selectedId.toStdString(), text.toStdString(), replyToMessageId.toStdString());
    refresh();
    return queued;
}

int ChatPageController::messageIndex(const QString& messageId) const
{
    return m_messageIndexes.value(messageId, -1);
}

void ChatPageController::refresh()
{
    if (!m_viewModel)
    {
        return;
    }

    auto data = m_viewModel->snapshot(m_selectedId.toStdString());
    if (data.ready && !m_pendingContactId.isEmpty())
    {
        const auto contactId = std::exchange(m_pendingContactId, {});
        const auto contactName = std::exchange(m_pendingContactName, {});
        const auto id = m_viewModel->openContact(contactId.toStdString(), contactName.toStdString());
        if (!id.empty())
        {
            m_selectedId = QString::fromStdString(id);
            m_revealSelected = true;
        }
        data = m_viewModel->snapshot(m_selectedId.toStdString());
    }

    if (m_selectedId.isEmpty() && !data.directory.mostRecentConversationId.empty())
    {
        m_selectedId = QString::fromStdString(data.directory.mostRecentConversationId);
        m_revealSelected = true;
        data = m_viewModel->snapshot(m_selectedId.toStdString());
    }

    const auto error = displayError(data.error);
    if (m_ready != data.ready || m_error != error)
    {
        m_ready = data.ready;
        m_error = error;
        emit statusChanged();
    }

    QVariantList groupOptions;
    QVariantList rows;
    QVariantMap currentConversation;
    const auto appendSection = [&](const QString& groupId, const QString& name,
                                   const std::vector<model::ChatConversation>& conversations) {
        QVariantMap heading{{QStringLiteral("id"), groupId}, {QStringLiteral("name"), name}};
        groupOptions.append(heading);
        if (groupId.isEmpty() && conversations.empty())
        {
            return;
        }

        QVariantList children;
        for (const auto& conversation : conversations)
        {
            const auto id = QString::fromStdString(conversation.id);
            const auto row = conversationRow(conversation, id);
            children.append(row);
            if (m_selectedId == id)
            {
                currentConversation = row;
                if (m_revealSelected)
                {
                    m_collapsedGroups.remove(groupId);
                }
            }
        }
        const bool collapsed = m_collapsedGroups.contains(groupId);
        heading.insert(QStringLiteral("isGroup"), true);
        heading.insert(QStringLiteral("collapsed"), collapsed);
        rows.append(heading);
        if (!collapsed)
        {
            rows.append(children);
        }
    };
    for (const auto& section : data.directory.groups)
    {
        appendSection(QString::fromStdString(section.group.id), QString::fromStdString(section.group.name),
                      section.conversations);
    }
    appendSection({}, tr("Ungrouped"), data.directory.ungrouped);
    m_revealSelected = false;

    QHash<QString, int> messageIndexes;
    for (int index = 0; const auto& message : data.messages)
    {
        messageIndexes.insert(QString::fromStdString(message.id), index++);
    }

    QVariantList messages;
    QDate previousDate;
    for (const auto& message : data.messages)
    {
        const auto time = QDateTime::fromMSecsSinceEpoch(message.timestampMs);
        const auto day = time.date();
        const auto dayLabel = day == previousDate ? QString{}
            : day == QDate::currentDate() ? tr("Today") : QLocale().toString(day, QLocale::ShortFormat);
        const auto& replyPreview = message.replyPreview;
        messages.append(QVariantMap{
            {QStringLiteral("id"), QString::fromStdString(message.id)},
            {QStringLiteral("text"), QString::fromStdString(message.text)},
            {QStringLiteral("day"), dayLabel},
            {QStringLiteral("time"), time.time().toString(QStringLiteral("HH:mm"))},
            {QStringLiteral("outgoing"), message.outgoing},
            {QStringLiteral("delivered"), message.delivered},
            {QStringLiteral("replyToMessageId"), QString::fromStdString(message.replyToMessageId)},
            {QStringLiteral("replyAvailable"), replyPreview.has_value()},
            {QStringLiteral("replyText"), replyPreview ? QString::fromStdString(replyPreview->text) : QString{}},
            {QStringLiteral("replyOutgoing"), replyPreview && replyPreview->outgoing}
        });
        previousDate = day;
    }

    if (m_groupOptions != groupOptions)
    {
        m_groupOptions = std::move(groupOptions);
        emit groupOptionsChanged();
    }
    if (m_conversation != currentConversation)
    {
        m_conversation = std::move(currentConversation);
        emit conversationChanged();
    }
    if (m_rows != rows)
    {
        m_rows = std::move(rows);
        emit rowsChanged();
    }
    if (m_messages != messages)
    {
        m_messages = std::move(messages);
        m_messageIndexes = std::move(messageIndexes);
        emit messagesChanged();
    }

    const auto connectionState = toUiState(data.connection.state);
    const auto connectionError = displayError(data.connection.error);
    const auto invite = QString::fromStdString(data.connection.invite);
    const auto answer = QString::fromStdString(data.connection.answer);
    const auto local = QString::fromStdString(data.connection.localFingerprint);
    const auto remote = QString::fromStdString(data.connection.remoteFingerprint);
    if (m_connectionState != connectionState || m_connectionError != connectionError ||
        m_invite != invite || m_answer != answer || m_localFingerprint != local ||
        m_remoteFingerprint != remote)
    {
        m_connectionState = connectionState;
        m_connectionError = connectionError;
        m_invite = invite;
        m_answer = answer;
        m_localFingerprint = local;
        m_remoteFingerprint = remote;
        emit connectionChanged();
    }
}
