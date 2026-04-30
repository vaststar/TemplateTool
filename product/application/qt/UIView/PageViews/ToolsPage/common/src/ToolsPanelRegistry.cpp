#include "ToolsPage/common/ToolsPanelRegistry.h"

#include <commonHead/viewModels/ToolsViewModel/IToolsModel.h>

ToolsPanelRegistry::ToolsPanelRegistry(QObject* parent)
    : QObject(parent)
{
    using PanelType = commonHead::viewModels::model::ToolPanelType;
    registerPanel(static_cast<int>(PanelType::Base64),          QStringLiteral("UIView/PageViews/ToolsPage/textprocess/qml/Base64Panel.qml"));
    registerPanel(static_cast<int>(PanelType::Json),            QStringLiteral("UIView/PageViews/ToolsPage/textprocess/qml/JsonPanel.qml"));
    registerPanel(static_cast<int>(PanelType::Timestamp),       QStringLiteral("UIView/PageViews/ToolsPage/textprocess/qml/TimestampPanel.qml"));
    registerPanel(static_cast<int>(PanelType::Uuid),            QStringLiteral("UIView/PageViews/ToolsPage/generators/qml/UuidPanel.qml"));
    registerPanel(static_cast<int>(PanelType::NetworkProxy),    QStringLiteral("UIView/PageViews/ToolsPage/network/qml/NetworkProxyPanel.qml"));
    registerPanel(static_cast<int>(PanelType::Screenshot),      QStringLiteral("UIView/PageViews/ToolsPage/screenshot/qml/ScreenshotPanel.qml"));
    registerPanel(static_cast<int>(PanelType::ScreenRecording), QStringLiteral("UIView/PageViews/ToolsPage/recording/qml/RecordingPanel.qml"));
}

void ToolsPanelRegistry::registerPanel(int panelType, const QString& qmlSource)
{
    m_panels.append({panelType, QStringLiteral("qrc:/qt/qml/") + qmlSource});
}

QStringList ToolsPanelRegistry::entries() const
{
    QStringList list;
    list.reserve(m_panels.size());
    for (const auto& p : m_panels)
    {
        list.append(p.second);
    }
    return list;
}

int ToolsPanelRegistry::indexOfPanel(int panelType) const
{
    for (int i = 0; i < m_panels.size(); ++i)
    {
        if (m_panels[i].first == panelType)
        {
            return i;
        }
    }
    return -1;
}
