#pragma once

#include <QObject>
#include <QStringList>
#include <QVector>
#include <QPair>
#include <QtQml>

class MainWindowContentPageRegistry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList entries READ entries NOTIFY entriesChanged)
    QML_ELEMENT
public:
    explicit MainWindowContentPageRegistry(QObject* parent = nullptr);

    QStringList entries() const;
    Q_INVOKABLE int indexOfPage(int pageId) const;

signals:
    void entriesChanged();

private:
    void registerPage(int pageId, const QString& qmlSource);
    QVector<QPair<int, QString>> m_pages;
};
