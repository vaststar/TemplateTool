#include "ToolsPage/common/ToolsPanelRegistry.h"

#include <QVariantMap>

#include <commonHead/viewModels/ToolsViewModel/IToolsModel.h>

ToolsPanelRegistry::ToolsPanelRegistry(QObject* parent)
    : QObject(parent)
{
    using PanelType = commonHead::viewModels::model::ToolPanelType;
    // preload=true: panel is constructed when ToolsPage is loaded.
    // preload=false (default): panel is constructed the first time the user selects it,
    // then kept alive for subsequent visits.
    registerPanel(static_cast<int>(PanelType::Base64),          QStringLiteral("UIView/PageViews/ToolsPage/textprocess/qml/Base64Panel.qml"));
    registerPanel(static_cast<int>(PanelType::Json),            QStringLiteral("UIView/PageViews/ToolsPage/textprocess/qml/JsonPanel.qml"));
    registerPanel(static_cast<int>(PanelType::Timestamp),       QStringLiteral("UIView/PageViews/ToolsPage/textprocess/qml/TimestampPanel.qml"));
    registerPanel(static_cast<int>(PanelType::Uuid),            QStringLiteral("UIView/PageViews/ToolsPage/generators/qml/UuidPanel.qml"));
    registerPanel(static_cast<int>(PanelType::NetworkProxy),    QStringLiteral("UIView/PageViews/ToolsPage/network/qml/NetworkProxyPanel.qml"));
    registerPanel(static_cast<int>(PanelType::Screenshot),      QStringLiteral("UIView/PageViews/ToolsPage/screenshot/qml/ScreenshotPanel.qml"));
    registerPanel(static_cast<int>(PanelType::ScreenRecording), QStringLiteral("UIView/PageViews/ToolsPage/recording/qml/RecordingPanel.qml"));
}

void ToolsPanelRegistry::registerPanel(int panelType, const QString& qmlSource, bool preload)
{
    m_panels.append({panelType, QStringLiteral("qrc:/qt/qml/") + qmlSource, preload});
}

QVariantList ToolsPanelRegistry::entries() const
{
    QVariantList list;
    list.reserve(m_panels.size());
    for (const auto& p : m_panels)
    {
        QVariantMap m;
        m.insert(QStringLiteral("source"),  p.source);
        m.insert(QStringLiteral("preload"), p.preload);
        list.append(m);
    }
    return list;
}

int ToolsPanelRegistry::indexOfPanel(int panelType) const
{
    for (int i = 0; i < m_panels.size(); ++i)
    {
        if (m_panels[i].panelType == panelType)
        {
            return i;
        }
    }
    return -1;
}
