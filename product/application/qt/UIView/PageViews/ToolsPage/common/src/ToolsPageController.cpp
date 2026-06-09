#include "ToolsPage/common/ToolsPageController.h"
#include "ToolsPage/common/ToolsTreeModel.h"
#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

#include <functional>

#include <commonHead/viewModels/ToolsViewModel/IToolsViewModel.h>
#include <commonHead/viewModels/ToolsViewModel/IToolsModel.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>

#include "ViewModelSingalEmitter/ToolsViewModelEmitter.h"

ToolsPageController::ToolsPageController(QObject* parent)
    : UIViewController(parent)
    , m_viewModelEmitter(std::make_shared<UIVMSignalEmitter::ToolsViewModelEmitter>())
    , m_panelRegistry(new ToolsPanelRegistry(this))
{
    UIVIEW_LOG_DEBUG("create ToolsPageController");
}

void ToolsPageController::init()
{
    UIVIEW_LOG_DEBUG("ToolsPageController::init");

    // Connect signals from ViewModel emitter
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::ToolsViewModelEmitter::signals_onToolsTreeReady,
            this, &ToolsPageController::onToolsTreeReady);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::ToolsViewModelEmitter::signals_onToolsNodesAdded,
            this, &ToolsPageController::onToolsNodesAdded);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::ToolsViewModelEmitter::signals_onToolsNodesUpdated,
            this, &ToolsPageController::onToolsNodesUpdated);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::ToolsViewModelEmitter::signals_onToolsNodesRemoved,
            this, &ToolsPageController::onToolsNodesRemoved);

    m_treeModel = new ToolsTreeModel(this);
    emit treeModelChanged();

    m_toolsViewModel = getAppContext()->getViewModelFactory()->createToolsViewModelInstance();
    m_toolsViewModel->registerCallback(m_viewModelEmitter);
    UIVIEW_LOG_DEBUG("ToolsPageController::init ViewModel created, initializing");
    m_toolsViewModel->initViewModel();

    UIVIEW_LOG_DEBUG("ToolsPageController::init done");
}

ToolsTreeModel* ToolsPageController::getTreeModel() const
{
    return m_treeModel;
}

int ToolsPageController::getCurrentPanelType() const
{
    return m_currentPanelType;
}

QString ToolsPageController::getCurrentNodeId() const
{
    return m_currentNodeId;
}

ToolsPanelRegistry* ToolsPageController::panelRegistry() const
{
    return m_panelRegistry;
}

void ToolsPageController::selectNode(const QString& nodeId)
{
    if (nodeId.isEmpty() || nodeId == m_currentNodeId) {
        return;
    }
    const commonHead::viewModels::model::ToolPanelType panelType =
        panelTypeOf(nodeId.toStdString());
    if (panelType == commonHead::viewModels::model::ToolPanelType::None) {
        // Category nodes are not selectable.
        return;
    }

    m_currentNodeId = nodeId;
    emit currentNodeIdChanged();

    const int newPanelType = static_cast<int>(panelType);
    if (m_currentPanelType != newPanelType) {
        m_currentPanelType = newPanelType;
        emit currentPanelTypeChanged();
    }

    // Notify VM purely for future metrics; VM holds no selection state.
    if (m_toolsViewModel) {
        m_toolsViewModel->selectNode(nodeId.toStdString());
    }

    UIVIEW_LOG_DEBUG("ToolsPageController::selectNode: " << nodeId.toStdString()
                     << " -> panelType=" << newPanelType);
}

void ToolsPageController::onLanguageChanged()
{
    UIVIEW_LOG_DEBUG("ToolsPageController::onLanguageChanged");
    if (m_toolsViewModel) {
        m_toolsViewModel->reloadTree();
    }
    // Title refresh keeps node IDs stable, so selection survives without help.
}

void ToolsPageController::onToolsTreeReady()
{
    UIVIEW_LOG_DEBUG("ToolsPageController::onToolsTreeReady");
    if (!m_treeModel || !m_toolsViewModel) return;
    m_treeModel->resetFromTree(m_toolsViewModel->getToolsTree());
    ensureValidSelection();
}

void ToolsPageController::onToolsNodesAdded(
    const std::vector<commonHead::viewModels::model::ToolNodeData>& nodes)
{
    UIVIEW_LOG_DEBUG("ToolsPageController::onToolsNodesAdded count=" << nodes.size());
    if (!m_treeModel) return;
    m_treeModel->insertNodes(nodes);
    ensureValidSelection();
}

void ToolsPageController::onToolsNodesUpdated(
    const std::vector<commonHead::viewModels::model::ToolNodeData>& nodes)
{
    UIVIEW_LOG_DEBUG("ToolsPageController::onToolsNodesUpdated count=" << nodes.size());
    if (!m_treeModel) return;
    m_treeModel->updateNodes(nodes);
}

void ToolsPageController::onToolsNodesRemoved(const std::vector<std::string>& nodeIds)
{
    UIVIEW_LOG_DEBUG("ToolsPageController::onToolsNodesRemoved count=" << nodeIds.size());
    if (!m_treeModel) return;
    m_treeModel->removeNodes(nodeIds);
    ensureValidSelection();
}

commonHead::viewModels::model::ToolPanelType
ToolsPageController::panelTypeOf(const std::string& nodeId) const
{
    using commonHead::viewModels::model::ToolPanelType;
    if (nodeId.empty() || !m_treeModel) return ToolPanelType::None;
    const QModelIndex idx = m_treeModel->indexOfId(QString::fromStdString(nodeId));
    if (!idx.isValid()) return ToolPanelType::None;
    return static_cast<ToolPanelType>(
        m_treeModel->data(idx, ToolsTreeModel::PanelTypeRole).toInt());
}

std::string ToolsPageController::findFirstSelectableNodeId() const
{
    using commonHead::viewModels::model::ToolPanelType;
    if (!m_treeModel) return "";

    std::function<std::string(const QModelIndex&)> dfs;
    dfs = [&](const QModelIndex& parent) -> std::string {
        const int n = m_treeModel->rowCount(parent);
        for (int i = 0; i < n; ++i) {
            const QModelIndex idx = m_treeModel->index(i, 0, parent);
            const auto panelType = static_cast<ToolPanelType>(
                m_treeModel->data(idx, ToolsTreeModel::PanelTypeRole).toInt());
            if (panelType != ToolPanelType::None) {
                return m_treeModel->data(idx, ToolsTreeModel::NodeIdRole)
                    .toString().toStdString();
            }
            auto r = dfs(idx);
            if (!r.empty()) return r;
        }
        return "";
    };
    return dfs(QModelIndex());
}

void ToolsPageController::ensureValidSelection()
{
    using commonHead::viewModels::model::ToolPanelType;

    // Keep current selection if it still points at a valid leaf.
    if (!m_currentNodeId.isEmpty()) {
        const ToolPanelType pt = panelTypeOf(m_currentNodeId.toStdString());
        if (pt != ToolPanelType::None) {
            const int newPanelType = static_cast<int>(pt);
            if (m_currentPanelType != newPanelType) {
                m_currentPanelType = newPanelType;
                emit currentPanelTypeChanged();
            }
            return;
        }
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
        if (m_currentPanelType != 0) {
            m_currentPanelType = 0;
            emit currentPanelTypeChanged();
        }
    }
}
