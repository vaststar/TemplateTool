#include "PageViews/ToolsPage/common/include/ToolsPageController.h"
#include "PageViews/ToolsPage/common/include/ToolsTreeModel.h"
#include "LoggerDefine/LoggerDefine.h"

#include <commonHead/viewModels/ToolsViewModel/IToolsViewModel.h>
#include <commonHead/viewModels/ToolsViewModel/IToolsModel.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>

#include "ViewModelSingalEmitter/ToolsViewModelEmitter.h"

ToolsPageController::ToolsPageController(QObject* parent)
    : UIViewController(parent)
    , m_viewModelEmitter(std::make_shared<UIVMSignalEmitter::ToolsViewModelEmitter>())
{
    UIVIEW_LOG_DEBUG("create ToolsPageController");
}

void ToolsPageController::init()
{
    UIVIEW_LOG_DEBUG("ToolsPageController::init");

    // Connect signals from ViewModel emitter
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::ToolsViewModelEmitter::signals_onToolsTreeChanged,
            this, &ToolsPageController::onToolsTreeChanged);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::ToolsViewModelEmitter::signals_onToolsTreeStructureChanged,
            this, &ToolsPageController::onToolsTreeStructureChanged);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::ToolsViewModelEmitter::signals_onToolsTreeItemsUpdated,
            this, &ToolsPageController::onToolsTreeItemsUpdated);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::ToolsViewModelEmitter::signals_onToolsTreeItemUpdated,
            this, &ToolsPageController::onToolsTreeItemUpdated);
    connect(m_viewModelEmitter.get(), &UIVMSignalEmitter::ToolsViewModelEmitter::signals_onCurrentToolNodeChanged,
            this, &ToolsPageController::onCurrentToolNodeChanged);

    // Create ViewModel, init, and register callback
    m_toolsViewModel = getAppContext()->getViewModelFactory()->createToolsViewModelInstance();
    m_toolsViewModel->initViewModel();
    m_toolsViewModel->registerCallback(m_viewModelEmitter);

    // Load initial data
    m_treeModel = new ToolsTreeModel(this);
    m_treeModel->setTree(m_toolsViewModel->getToolsTree());
    emit treeModelChanged();

    // Sync initial selection from ViewModel
    QString nodeId = QString::fromStdString(m_toolsViewModel->getCurrentNodeId());
    int panelType = static_cast<int>(m_toolsViewModel->getCurrentPanelType());
    if (!nodeId.isEmpty()) {
        onCurrentToolNodeChanged(nodeId, panelType);
    }

    UIVIEW_LOG_DEBUG("ToolsPageController::init done");
}

ToolsTreeModel* ToolsPageController::getTreeModel() const
{
    return m_treeModel;
}

QString ToolsPageController::getCurrentPanelQml() const
{
    return m_currentPanelQml;
}

QString ToolsPageController::getCurrentNodeId() const
{
    return m_currentNodeId;
}

void ToolsPageController::selectNode(const QString& nodeId)
{
    if (m_toolsViewModel) {
        m_toolsViewModel->selectNode(nodeId.toStdString());
    }
}

void ToolsPageController::onLanguageChanged()
{
    UIVIEW_LOG_DEBUG("ToolsPageController::onLanguageChanged");
    if (m_toolsViewModel) {
        m_toolsViewModel->reloadTree();
    }
}

void ToolsPageController::onToolsTreeChanged(const std::shared_ptr<commonHead::viewModels::model::IToolsTree>& tree)
{
    if (m_treeModel) {
        m_treeModel->setTree(tree);
        emit treeModelChanged();
    }
}

void ToolsPageController::onToolsTreeStructureChanged(
    const commonHead::viewModels::model::ToolsTreeNodeChange& change)
{
    UIVIEW_LOG_DEBUG("ToolsPageController::onToolsTreeStructureChanged");
    if (m_treeModel) {
        m_treeModel->applyStructureChange(change);
    }
}

void ToolsPageController::onToolsTreeItemsUpdated()
{
    UIVIEW_LOG_DEBUG("ToolsPageController::onToolsTreeItemsUpdated");
    if (m_treeModel) {
        m_treeModel->notifyAllItemsChanged();
    }
}

void ToolsPageController::onToolsTreeItemUpdated(const QString& nodeId)
{
    UIVIEW_LOG_DEBUG("ToolsPageController::onToolsTreeItemUpdated: " << nodeId.toStdString());
    if (m_treeModel) {
        m_treeModel->notifyItemChanged(nodeId.toStdString());
    }
}

void ToolsPageController::onCurrentToolNodeChanged(const QString& nodeId, int panelType)
{
    m_currentNodeId = nodeId;
    emit currentNodeIdChanged();

    QString newPanelQml = mapPanelTypeToQml(panelType);
    if (m_currentPanelQml != newPanelQml) {
        m_currentPanelQml = newPanelQml;
        emit currentPanelQmlChanged();
    }

    UIVIEW_LOG_DEBUG("onCurrentToolNodeChanged: " << nodeId.toStdString() << " -> " << m_currentPanelQml.toStdString());
}

QString ToolsPageController::mapPanelTypeToQml(int panelType) const
{
    using PanelType = commonHead::viewModels::model::ToolPanelType;

    switch (static_cast<PanelType>(panelType)) {
    case PanelType::Base64:
        return QStringLiteral("Base64Panel.qml");
    case PanelType::Json:
        return QStringLiteral("JsonPanel.qml");
    case PanelType::Timestamp:
        return QStringLiteral("TimestampPanel.qml");
    case PanelType::Uuid:
        return QStringLiteral("UuidPanel.qml");
    case PanelType::NetworkProxy:
        return QStringLiteral("NetworkProxyPanel.qml");
    default:
        return QString();
    }
}
