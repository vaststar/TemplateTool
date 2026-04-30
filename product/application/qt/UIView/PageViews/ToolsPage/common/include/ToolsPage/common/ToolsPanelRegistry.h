#pragma once

#include <QObject>
#include <QStringList>
#include <QVector>
#include <QPair>
#include <QtQml>

class ToolsPanelRegistry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList entries READ entries NOTIFY entriesChanged)
    QML_ELEMENT
public:
    explicit ToolsPanelRegistry(QObject* parent = nullptr);

    QStringList entries() const;
    Q_INVOKABLE int indexOfPanel(int panelType) const;

signals:
    void entriesChanged();

private:
    void registerPanel(int panelType, const QString& qmlSource);
    QVector<QPair<int, QString>> m_panels;
};
