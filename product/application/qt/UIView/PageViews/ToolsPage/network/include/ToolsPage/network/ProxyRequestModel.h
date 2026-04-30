#pragma once

#include <QAbstractListModel>
#include <QJsonObject>
#include <QJsonArray>
#include <QSortFilterProxyModel>
#include <QVector>
#include <QtQml>

/**
 * @brief List model holding captured HTTP(S) requests reported by the proxy addon.
 *
 * Each row represents a request/response pair identified by a unique flow_id.
 * The addon sends "request" events first, then "response" events that update
 * the same row with status_code, response headers, body, duration, etc.
 */
class ProxyRequestModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        FlowIdRole = Qt::UserRole + 1,
        MethodRole,
        UrlRole,
        HostRole,
        PathRole,
        StatusCodeRole,
        ContentTypeRole,
        ContentLengthRole,
        DurationRole,
        TimestampRole,
        ProcessNameRole,
        IsHttpsRole,
        IsWebSocketRole,
        IsInterceptedRole,
        FullDataRole,       // QJsonObject serialised to string
    };

    explicit ProxyRequestModel(QObject* parent = nullptr);

    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Public API for the controller
    void addOrUpdateRequest(const QJsonObject& msg);
    QJsonObject getRequestAt(int row) const;
    void clear();

    /// Returns sorted unique process names from captured requests (for QML filter popup)
    Q_INVOKABLE QStringList uniqueProcessNames() const;

private:
    struct RequestEntry {
        QString flowId;
        QString method;
        QString url;
        QString host;
        QString path;
        int     statusCode    = 0;
        QString contentType;
        qint64  contentLength = 0;
        double  duration      = 0.0;       // seconds
        QString timestamp;
        QString processName;
        bool    isHttps       = false;
        bool    isWebSocket   = false;
        bool    isIntercepted = false;

        QJsonObject fullData;               // raw JSON from addon
    };

    QVector<RequestEntry> m_entries;
    QHash<QString, int>   m_flowIdIndex;    // flow_id -> row
};

/**
 * @brief Filter-proxy that sits between ProxyRequestModel and the QML ListView.
 *
 * Supports filtering by URL substring, HTTP method, status code range, and
 * content type. Exposed as a QML_ELEMENT so it can be instantiated in QML.
 */
class ProxyFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString filterUrl    READ filterUrl    WRITE setFilterUrl    NOTIFY filterUrlChanged)
    Q_PROPERTY(QString filterMethod READ filterMethod WRITE setFilterMethod NOTIFY filterMethodChanged)
    Q_PROPERTY(QString filterStatus READ filterStatus WRITE setFilterStatus NOTIFY filterStatusChanged)
    Q_PROPERTY(QString filterContentType READ filterContentType WRITE setFilterContentType NOTIFY filterContentTypeChanged)
    Q_PROPERTY(QString filterProcess READ filterProcess WRITE setFilterProcess NOTIFY filterProcessChanged)

public:
    explicit ProxyFilterModel(QObject* parent = nullptr);

    QString filterUrl() const;
    QString filterMethod() const;
    QString filterStatus() const;
    QString filterContentType() const;
    QString filterProcess() const;

    void setFilterUrl(const QString& v);
    void setFilterMethod(const QString& v);
    void setFilterStatus(const QString& v);
    void setFilterContentType(const QString& v);
    void setFilterProcess(const QString& v);

signals:
    void filterUrlChanged();
    void filterMethodChanged();
    void filterStatusChanged();
    void filterContentTypeChanged();
    void filterProcessChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    QString m_filterUrl;
    QString m_filterMethod;
    QString m_filterStatus;
    QString m_filterContentType;
    QString m_filterProcess;
};
