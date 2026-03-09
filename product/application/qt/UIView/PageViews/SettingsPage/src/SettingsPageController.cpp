#include "PageViews/SettingsPage/include/SettingsPageController.h"
#include "LoggerDefine/LoggerDefine.h"

#include <commonHead/viewModels/SettingsViewModel/ISettingsViewModel.h>
#include <commonHead/viewModels/SettingsViewModel/ISettingsModel.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>

SettingsPageController::SettingsPageController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create SettingsPageController");
}

void SettingsPageController::init()
{
    UIVIEW_LOG_DEBUG("SettingsPageController::init");

    m_settingsViewModel = getAppContext()->getViewModelFactory()->createSettingsViewModelInstance();
    m_settingsViewModel->initViewModel();

    m_treeModel = new SettingsTreeModel(this);
    m_treeModel->setViewModel(m_settingsViewModel);
    emit treeModelChanged();

    selectFirstNode();

    UIVIEW_LOG_DEBUG("SettingsPageController::init done");
}

SettingsTreeModel* SettingsPageController::getTreeModel() const
{
    return m_treeModel;
}

QString SettingsPageController::getCurrentPanelQml() const
{
    return m_currentPanelQml;
}

QString SettingsPageController::getCurrentNodeId() const
{
    return m_currentNodeId;
}

void SettingsPageController::selectNode(const QString& nodeId)
{
    if (!m_settingsViewModel)
        return;

    auto tree = m_settingsViewModel->getSettingsTree();
    auto node = tree->findNodeById(nodeId.toStdString());
    if (!node)
        return;

    auto nodeData = node->getNodeData();

    // Update current node id
    m_currentNodeId = nodeId;
    emit currentNodeIdChanged();

    // Only switch panel if node has one
    if (nodeData.panelType != commonHead::viewModels::model::SettingsPanelType::None) {
        m_currentPanelQml = mapPanelTypeToQml(static_cast<int>(nodeData.panelType));
        emit currentPanelQmlChanged();
    }

    UIVIEW_LOG_DEBUG("selectNode: " << nodeId.toStdString() << " -> " << m_currentPanelQml.toStdString());
}

QString SettingsPageController::mapPanelTypeToQml(int panelType) const
{
    using PanelType = commonHead::viewModels::model::SettingsPanelType;

    switch (static_cast<PanelType>(panelType)) {
    case PanelType::Appearance:
        return QStringLiteral("AppearanceSettingsPanel.qml");
    case PanelType::Language:
        return QStringLiteral("LanguageSettingsPanel.qml");
    default:
        return QString();
    }
}

void SettingsPageController::selectFirstNode()
{
    if (!m_settingsViewModel)
        return;

    auto tree = m_settingsViewModel->getSettingsTree();
    auto root = tree->getRoot();
    if (!root || root->getChildCount() == 0)
        return;

    // Select first top-level node (first child of virtual root, i.e., General)
    auto firstNode = root->getChild(0);
    if (firstNode) {
        auto data = firstNode->getNodeData();
        m_currentNodeId = QString::fromStdString(data.nodeId);
        emit currentNodeIdChanged();

        // If first node has a panel, load it
        if (data.panelType != commonHead::viewModels::model::SettingsPanelType::None) {
            m_currentPanelQml = mapPanelTypeToQml(static_cast<int>(data.panelType));
            emit currentPanelQmlChanged();
        }
    }
}
