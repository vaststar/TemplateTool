#include "PageViews/ToolsPage/include/ToolsPageController.h"
#include "PageViews/ToolsPage/include/ToolsTreeModel.h"
#include "LoggerDefine/LoggerDefine.h"

#include <commonHead/viewModels/ToolsViewModel/IToolsViewModel.h>
#include <commonHead/viewModels/ToolsViewModel/IToolsModel.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>

ToolsPageController::ToolsPageController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create ToolsPageController");
}

void ToolsPageController::init()
{
    UIVIEW_LOG_DEBUG("ToolsPageController::init");

    m_toolsViewModel = getAppContext()->getViewModelFactory()->createToolsViewModelInstance();
    m_toolsViewModel->initViewModel();

    m_treeModel = new ToolsTreeModel(this);
    m_treeModel->setTree(m_toolsViewModel->getToolsTree());
    emit treeModelChanged();

    selectFirstNode();

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
    if (!m_toolsViewModel)
        return;

    auto tree = m_toolsViewModel->getToolsTree();
    auto node = tree->findNodeById(nodeId.toStdString());
    if (!node)
        return;

    auto nodeData = node->getNodeData();

    // Update current node id
    m_currentNodeId = nodeId;
    emit currentNodeIdChanged();

    // Only switch panel if node has one (not a category)
    if (nodeData.panelType != commonHead::viewModels::model::ToolPanelType::None) {
        m_currentPanelQml = mapPanelTypeToQml(static_cast<int>(nodeData.panelType));
        emit currentPanelQmlChanged();
    }

    UIVIEW_LOG_DEBUG("selectNode: " << nodeId.toStdString() << " -> " << m_currentPanelQml.toStdString());
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
    default:
        return QString();
    }
}

void ToolsPageController::selectFirstNode()
{
    if (!m_toolsViewModel)
        return;

    auto tree = m_toolsViewModel->getToolsTree();
    auto root = tree->getRoot();
    if (!root || root->getChildCount() == 0)
        return;

    // Find first tool node (not category) by DFS
    std::function<std::string(const commonHead::viewModels::model::ToolsTreeNodePtr&)> findFirst;
    findFirst = [&findFirst](const commonHead::viewModels::model::ToolsTreeNodePtr& node) -> std::string {
        if (!node)
            return "";
        
        auto data = node->getNodeData();
        if (data.panelType != commonHead::viewModels::model::ToolPanelType::None) {
            return data.nodeId;
        }
        
        for (std::size_t i = 0; i < node->getChildCount(); ++i) {
            auto child = node->getChild(i);
            auto result = findFirst(child);
            if (!result.empty())
                return result;
        }
        return "";
    };

    std::string firstToolId = findFirst(root);
    if (!firstToolId.empty()) {
        selectNode(QString::fromStdString(firstToolId));
    }
}
