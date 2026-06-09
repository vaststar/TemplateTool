#include "SettingsPage/SettingsPageController.h"
#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

#include <functional>

#include <commonHead/viewModels/SettingsViewModel/ISettingsViewModel.h>
#include <commonHead/viewModels/SettingsViewModel/ISettingsModel.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>

#include "ViewModelSingalEmitter/SettingsViewModelEmitter.h"

SettingsPageController::SettingsPageController(QObject* parent)
    : UIViewController(parent)
    , m_viewModelEmitter(std::make_shared<UIVMSignalEmitter::SettingsViewModelEmitter>())
{
    UIVIEW_LOG_DEBUG("create SettingsPageController");
}

void SettingsPageController::init()
{
    UIVIEW_LOG_DEBUG("SettingsPageController::init");

    // Connect signals from ViewModel emitter
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SettingsViewModelEmitter::signals_onSettingsTreeReady,
            this, &SettingsPageController::onSettingsTreeReady);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SettingsViewModelEmitter::signals_onSettingsNodesAdded,
            this, &SettingsPageController::onSettingsNodesAdded);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SettingsViewModelEmitter::signals_onSettingsNodesUpdated,
            this, &SettingsPageController::onSettingsNodesUpdated);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SettingsViewModelEmitter::signals_onSettingsNodesRemoved,
            this, &SettingsPageController::onSettingsNodesRemoved);

    m_treeModel = new SettingsTreeModel(this);
    emit treeModelChanged();

    m_settingsViewModel = getAppContext()->getViewModelFactory()->createSettingsViewModelInstance();
    m_settingsViewModel->registerCallback(m_viewModelEmitter);
    UIVIEW_LOG_DEBUG("SettingsPageController::init ViewModel created, initializing");
    m_settingsViewModel->initViewModel();
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
    if (nodeId.isEmpty() || nodeId == m_currentNodeId) {
        return;
    }
    const commonHead::viewModels::model::SettingsPanelType panelType =
        panelTypeOf(nodeId.toStdString());
    if (panelType == commonHead::viewModels::model::SettingsPanelType::None) {
        // Category nodes are not selectable.
        return;
    }

    m_currentNodeId = nodeId;
    emit currentNodeIdChanged();

    QString newPanelQml = mapPanelTypeToQml(panelType);
    if (m_currentPanelQml != newPanelQml) {
        m_currentPanelQml = newPanelQml;
        emit currentPanelQmlChanged();
    }

    // Notify VM purely for future metrics; VM holds no selection state.
    if (m_settingsViewModel) {
        m_settingsViewModel->selectNode(nodeId.toStdString());
    }

    UIVIEW_LOG_DEBUG("SettingsPageController::selectNode: " << nodeId.toStdString()
                     << " -> " << m_currentPanelQml.toStdString());
}

void SettingsPageController::onLanguageChanged()
{
    UIVIEW_LOG_DEBUG("SettingsPageController::onLanguageChanged");
    if (m_settingsViewModel) {
        m_settingsViewModel->reloadTree();
    }
    // Title refresh keeps node IDs stable, so selection survives without help.
}

void SettingsPageController::onSettingsTreeReady()
{
    UIVIEW_LOG_DEBUG("SettingsPageController::onSettingsTreeReady");
    if (!m_treeModel || !m_settingsViewModel) return;
    m_treeModel->resetFromTree(m_settingsViewModel->getSettingsTree());
    ensureValidSelection();
}

void SettingsPageController::onSettingsNodesAdded(
    const std::vector<commonHead::viewModels::model::SettingsNodeData>& nodes)
{
    UIVIEW_LOG_DEBUG("SettingsPageController::onSettingsNodesAdded count=" << nodes.size());
    if (!m_treeModel) return;
    m_treeModel->insertNodes(nodes);
    ensureValidSelection();
}

void SettingsPageController::onSettingsNodesUpdated(
    const std::vector<commonHead::viewModels::model::SettingsNodeData>& nodes)
{
    UIVIEW_LOG_DEBUG("SettingsPageController::onSettingsNodesUpdated count=" << nodes.size());
    if (!m_treeModel) return;
    m_treeModel->updateNodes(nodes);
}

void SettingsPageController::onSettingsNodesRemoved(const std::vector<std::string>& nodeIds)
{
    UIVIEW_LOG_DEBUG("SettingsPageController::onSettingsNodesRemoved count=" << nodeIds.size());
    if (!m_treeModel) return;
    m_treeModel->removeNodes(nodeIds);
    ensureValidSelection();
}

QString SettingsPageController::mapPanelTypeToQml(
    commonHead::viewModels::model::SettingsPanelType panelType) const
{
    switch (panelType) {
    case commonHead::viewModels::model::SettingsPanelType::Appearance:
        return QStringLiteral("AppearanceSettingsPanel.qml");
    case commonHead::viewModels::model::SettingsPanelType::Language:
        return QStringLiteral("LanguageSettingsPanel.qml");
    default:
        return QString();
    }
}

commonHead::viewModels::model::SettingsPanelType
SettingsPageController::panelTypeOf(const std::string& nodeId) const
{
    using commonHead::viewModels::model::SettingsPanelType;
    if (nodeId.empty() || !m_treeModel) return SettingsPanelType::None;
    const QModelIndex idx = m_treeModel->indexOfId(QString::fromStdString(nodeId));
    if (!idx.isValid()) return SettingsPanelType::None;
    return static_cast<SettingsPanelType>(
        m_treeModel->data(idx, SettingsTreeModel::PanelTypeRole).toInt());
}

std::string SettingsPageController::findFirstSelectableNodeId() const
{
    using commonHead::viewModels::model::SettingsPanelType;
    if (!m_treeModel) return "";

    std::function<std::string(const QModelIndex&)> dfs;
    dfs = [&](const QModelIndex& parent) -> std::string {
        const int n = m_treeModel->rowCount(parent);
        for (int i = 0; i < n; ++i) {
            const QModelIndex idx = m_treeModel->index(i, 0, parent);
            const auto panelType = static_cast<SettingsPanelType>(
                m_treeModel->data(idx, SettingsTreeModel::PanelTypeRole).toInt());
            if (panelType != SettingsPanelType::None) {
                return m_treeModel->data(idx, SettingsTreeModel::NodeIdRole)
                    .toString().toStdString();
            }
            auto r = dfs(idx);
            if (!r.empty()) return r;
        }
        return "";
    };
    return dfs(QModelIndex());
}

void SettingsPageController::ensureValidSelection()
{
    using commonHead::viewModels::model::SettingsPanelType;

    // Keep current selection if it still points at a valid leaf.
    if (!m_currentNodeId.isEmpty() &&
        panelTypeOf(m_currentNodeId.toStdString()) != SettingsPanelType::None) {
        // Panel mapping is stable for a given nodeId in practice, but in case
        // the panelType of this node changed during reload, refresh the QML path.
        QString newPanelQml = mapPanelTypeToQml(panelTypeOf(m_currentNodeId.toStdString()));
        if (m_currentPanelQml != newPanelQml) {
            m_currentPanelQml = newPanelQml;
            emit currentPanelQmlChanged();
        }
        return;
    }

    // Selection no longer valid: fall back to the first selectable node.
    const QString old = m_currentNodeId;
    m_currentNodeId.clear();  // bypass the no-op guard in selectNode()
    const QString fallback = QString::fromStdString(findFirstSelectableNodeId());
    if (!fallback.isEmpty()) {
        selectNode(fallback);
    } else if (!old.isEmpty()) {
        // No selectable node anywhere; let QML clear its highlight.
        emit currentNodeIdChanged();
        if (!m_currentPanelQml.isEmpty()) {
            m_currentPanelQml.clear();
            emit currentPanelQmlChanged();
        }
    }
}
