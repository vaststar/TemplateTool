#pragma once

#include <memory>

#include <QHash>
#include <QSet>
#include <QString>
#include <QtQml>
#include <QVariantList>
#include <QVariantMap>

#include <UIViewCore/UIViewController.h>

namespace commonHead::viewModels {
class IChatViewModel;
}

namespace UIViewModelSignalBridge {
class ChatViewModelEmitter;
}

class ChatPageController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool ready READ ready NOTIFY statusChanged)
    Q_PROPERTY(QString error READ error NOTIFY statusChanged)
    Q_PROPERTY(QVariantList rows READ rows NOTIFY rowsChanged)
    Q_PROPERTY(QVariantList groupOptions READ groupOptions NOTIFY groupOptionsChanged)
    Q_PROPERTY(QVariantMap conversation READ conversation NOTIFY conversationChanged)
    Q_PROPERTY(QVariantList messages READ messages NOTIFY messagesChanged)
    Q_PROPERTY(ConnectionState connectionState READ connectionState NOTIFY connectionChanged)
    Q_PROPERTY(QString connectionError READ connectionError NOTIFY connectionChanged)
    Q_PROPERTY(bool inviteAvailable READ inviteAvailable NOTIFY connectionChanged)
    Q_PROPERTY(bool answerAvailable READ answerAvailable NOTIFY connectionChanged)
    Q_PROPERTY(QString localFingerprint READ localFingerprint NOTIFY connectionChanged)
    Q_PROPERTY(QString remoteFingerprint READ remoteFingerprint NOTIFY connectionChanged)

public:
    enum ConnectionState { Disconnected, PreparingInvite, AwaitingAnswer, Connecting,
                           VerifyPeer, Connected, Failed };
    Q_ENUM(ConnectionState)

    explicit ChatPageController(QObject* parent = nullptr);
    ~ChatPageController() override;

    bool ready() const { return m_ready; }
    QString error() const { return m_error; }
    QVariantList rows() const { return m_rows; }
    QVariantList groupOptions() const { return m_groupOptions; }
    QVariantMap conversation() const { return m_conversation; }
    QVariantList messages() const { return m_messages; }
    ConnectionState connectionState() const { return m_connectionState; }
    QString connectionError() const { return m_connectionError; }
    bool inviteAvailable() const { return !m_invite.isEmpty(); }
    bool answerAvailable() const { return !m_answer.isEmpty(); }
    QString localFingerprint() const { return m_localFingerprint; }
    QString remoteFingerprint() const { return m_remoteFingerprint; }

    void openContact(const QString& contactId, const QString& displayName);
    Q_INVOKABLE void selectConversation(const QString& conversationId);
    Q_INVOKABLE void toggleGroup(const QString& groupId);
    Q_INVOKABLE bool createGroup(const QString& name);
    Q_INVOKABLE bool renameGroup(const QString& groupId, const QString& name);
    Q_INVOKABLE void moveConversation(const QString& groupId);
    Q_INVOKABLE void deleteGroup(const QString& groupId);

    Q_INVOKABLE void createInvite();
    Q_INVOKABLE bool acceptInvite(const QString& text, bool linkSelectedContact);
    Q_INVOKABLE bool applyAnswer(const QString& text);
    Q_INVOKABLE bool copyInvite() const;
    Q_INVOKABLE bool copyAnswer() const;
    Q_INVOKABLE void verifyPeer();
    Q_INVOKABLE void disconnectPeer();
    Q_INVOKABLE bool sendText(const QString& text, const QString& replyToMessageId);
    Q_INVOKABLE int messageIndex(const QString& messageId) const;

signals:
    void statusChanged();
    void rowsChanged();
    void groupOptionsChanged();
    void conversationChanged();
    void messagesChanged();
    void connectionChanged();

protected:
    void init() override;
    void onLanguageChanged() override;

private:
    void refresh();

    std::shared_ptr<commonHead::viewModels::IChatViewModel> m_viewModel;
    std::shared_ptr<UIViewModelSignalBridge::ChatViewModelEmitter> m_emitter;
    QString m_selectedId;
    QString m_pendingContactId;
    QString m_pendingContactName;
    bool m_revealSelected = false;
    QSet<QString> m_collapsedGroups;

    bool m_ready = false;
    QString m_error;
    QVariantList m_rows;
    QVariantList m_groupOptions;
    QVariantMap m_conversation;
    QVariantList m_messages;
    QHash<QString, int> m_messageIndexes;
    ConnectionState m_connectionState = Disconnected;
    QString m_connectionError;
    QString m_invite;
    QString m_answer;
    QString m_localFingerprint;
    QString m_remoteFingerprint;
};
