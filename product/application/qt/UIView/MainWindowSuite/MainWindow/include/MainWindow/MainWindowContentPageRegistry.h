#pragma once

#include <QObject>
#include <QVariantList>
#include <QVector>
#include <QtQml>

class MainWindowContentPageRegistry : public QObject
{
    Q_OBJECT
    // Each entry is a QVariantMap with keys: "source" (QString), "preload" (bool).
    Q_PROPERTY(QVariantList entries READ entries NOTIFY entriesChanged)
    QML_ELEMENT
public:
    explicit MainWindowContentPageRegistry(QObject* parent = nullptr);

    QVariantList entries() const;
    Q_INVOKABLE int indexOfPage(int pageId) const;

signals:
    void entriesChanged();

private:
    struct PageEntry {
        int     pageId;
        QString source;
        bool    preload;
    };
    void registerPage(int pageId, const QString& qmlSource, bool preload = false);
    QVector<PageEntry> m_pages;
};
