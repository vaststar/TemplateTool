#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QJsonObject>
#include <QStringList>
#include <QVector>

/// Presentation of requests currently paused by the addon, independent of capture history.
class ProxyInterceptModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role {
        FlowIdRole = Qt::UserRole + 1,
        MethodRole,
        UrlRole,
        ActionPendingRole,
    };

    explicit ProxyInterceptModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int count() const;

    bool addOrUpdate(const QString& flowId, const QJsonObject& detail);
    bool contains(const QString& flowId) const;
    bool isActionPending(const QString& flowId) const;
    void setActionPending(const QString& flowId, bool pending);
    void remove(const QString& flowId);
    void clear();
    QStringList flowIds() const;

signals:
    void countChanged();

private:
    // Keep in sync with the addon's pending limit. Display strings are bounded too.
    static constexpr int kMaxPendingFlows = 128;
    static constexpr int kMaxFlowIdLength = 256;
    static constexpr int kMaxMethodLength = 32;
    static constexpr int kMaxUrlLength = 4096;

    struct Entry {
        QString flowId;
        QString method;
        QString url;
        bool actionPending = false;
    };

    QVector<Entry> m_entries;
    QHash<QString, int> m_flowIdIndex;
};
