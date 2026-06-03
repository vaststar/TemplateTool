#pragma once

#include <QObject>
#include <QVariantList>
#include <QVector>
#include <QtQml>

class ToolsPanelRegistry : public QObject
{
    Q_OBJECT
    // Each entry is a QVariantMap with keys: "source" (QString), "preload" (bool).
    Q_PROPERTY(QVariantList entries READ entries NOTIFY entriesChanged)
    QML_ELEMENT
public:
    explicit ToolsPanelRegistry(QObject* parent = nullptr);

    QVariantList entries() const;
    Q_INVOKABLE int indexOfPanel(int panelType) const;

signals:
    void entriesChanged();

private:
    struct PanelEntry {
        int     panelType;
        QString source;
        bool    preload;
    };
    void registerPanel(int panelType, const QString& qmlSource, bool preload = false);
    QVector<PanelEntry> m_panels;
};
