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
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SettingsViewModelEmitter::signals_onSettingsTreeChanged,
            this, &SettingsPageController::onSettingsTreeChanged);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SettingsViewModelEmitter::signals_onSettingsTreeStructureChanged,
            this, &SettingsPageController::onSettingsTreeStructureChanged);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SettingsViewModelEmitter::signals_onSettingsTreeItemsUpdated,
            this, &SettingsPageController::onSettingsTreeItemsUpdated);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::SettingsViewModelEmitter::signals_onSettingsTreeItemUpdated,
            this, &SettingsPageController::onSettingsTreeItemUpdated);

    // Create ViewModel, init, and register callback
    m_settingsViewModel = getAppContext()->getViewModelFactory()->createSettingsViewModelInstance();
    m_settingsViewModel->initViewModel();
    m_settingsViewModel->registerCallback(m_viewModelEmitter);

    // Load initial data
    m_treeModel = new SettingsTreeModel(this);
    m_treeModel->setTree(m_settingsViewModel->getSettingsTree());
    emit treeModelChanged();

    // Controller owns selection; pick the first selectable node.
    selectNode(QString::fromStdString(findFirstSelectableNodeId()));

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
    // L2 path keeps node IDs stable; selection survives without intervention.
    // L4 path goes through onSettingsTreeChanged, which calls ensureValidSelection().
}

void SettingsPageController::onSettingsTreeChanged(const std::shared_ptr<commonHead::viewModels::model::ISettingsTree>& tree)
{
    if (!m_treeModel) return;
    m_treeModel->setTree(tree);
    emit treeModelChanged();
    ensureValidSelection();
}

void SettingsPageController::onSettingsTreeStructureChanged(
    const commonHead::viewModels::model::SettingsTreeNodeChange& change)
{
    UIVIEW_LOG_DEBUG("SettingsPageController::onSettingsTreeStructureChanged");
    if (!m_treeModel) return;
    m_treeModel->applyStructureChange(change);
    ensureValidSelection();
}

void SettingsPageController::onSettingsTreeItemsUpdated()
{
    UIVIEW_LOG_DEBUG("SettingsPageController::onSettingsTreeItemsUpdated");
    if (m_treeModel) {
        m_treeModel->notifyAllItemsChanged();
    }
}

void SettingsPageController::onSettingsTreeItemUpdated(const QString& nodeId)
{
    UIVIEW_LOG_DEBUG("SettingsPageController::onSettingsTreeItemUpdated: " << nodeId.toStdString());
    if (m_treeModel) {
        m_treeModel->notifyItemChanged(nodeId.toStdString());
    }
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
    if (nodeId.empty() || !m_settingsViewModel) return SettingsPanelType::None;
    auto tree = m_settingsViewModel->getSettingsTree();
    if (!tree) return SettingsPanelType::None;
    auto node = tree->findNodeById(nodeId);
    if (!node) return SettingsPanelType::None;
    return node->getNodeData().panelType;
}

std::string SettingsPageController::findFirstSelectableNodeId() const
{
    if (!m_settingsViewModel) return "";
    auto tree = m_settingsViewModel->getSettingsTree();
    if (!tree) return "";
    auto root = tree->getRoot();
    if (!root) return "";

    std::function<std::string(const commonHead::viewModels::model::SettingsTreeNodePtr&)> dfs;
    dfs = [&dfs](const commonHead::viewModels::model::SettingsTreeNodePtr& node) -> std::string {
        if (!node) return "";
        auto data = node->getNodeData();
        if (data.panelType != commonHead::viewModels::model::SettingsPanelType::None) {
            return data.nodeId;
        }
        for (std::size_t i = 0; i < node->getChildCount(); ++i) {
            auto r = dfs(node->getChild(i));
            if (!r.empty()) return r;
        }
        return "";
    };
    return dfs(root);
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
