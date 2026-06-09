#pragma once

#include <QObject>
#include <QtQml>

#include <string>
#include <vector>

#include "UIViewBase/UIViewController.h"
#include "ToolsTreeModel.h"
#include "ToolsPanelRegistry.h"
#include "ViewModelSingalEmitter/ToolsViewModelEmitter.h"

namespace commonHead::viewModels {
    class IToolsViewModel;
}
namespace commonHead::viewModels::model {
    struct ToolNodeData;
    enum class ToolPanelType : std::uint8_t;
}

class ToolsPageController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(ToolsTreeModel* treeModel READ getTreeModel NOTIFY treeModelChanged)
    Q_PROPERTY(int currentPanelType READ getCurrentPanelType NOTIFY currentPanelTypeChanged)
    Q_PROPERTY(QString currentNodeId READ getCurrentNodeId NOTIFY currentNodeIdChanged)
    Q_PROPERTY(ToolsPanelRegistry* panelRegistry READ panelRegistry CONSTANT)

public:
    explicit ToolsPageController(QObject* parent = nullptr);

    ToolsTreeModel* getTreeModel() const;
    int getCurrentPanelType() const;
    QString getCurrentNodeId() const;

    ToolsPanelRegistry* panelRegistry() const;

    Q_INVOKABLE void selectNode(const QString& nodeId);

protected:
    void init() override;
    void onLanguageChanged() override;

signals:
    void treeModelChanged();
    void currentPanelTypeChanged();
    void currentNodeIdChanged();

private slots:
    void onToolsTreeReady();
    void onToolsNodesAdded(const std::vector<commonHead::viewModels::model::ToolNodeData>& nodes);
    void onToolsNodesUpdated(const std::vector<commonHead::viewModels::model::ToolNodeData>& nodes);
    void onToolsNodesRemoved(const std::vector<std::string>& nodeIds);

private:
    // Mirror-reading helpers (do NOT touch the VM's tree object).
    commonHead::viewModels::model::ToolPanelType panelTypeOf(const std::string& nodeId) const;
    std::string findFirstSelectableNodeId() const;
    void        ensureValidSelection();

private:
    std::shared_ptr<commonHead::viewModels::IToolsViewModel> m_toolsViewModel;
    std::shared_ptr<UIVMSignalEmitter::ToolsViewModelEmitter> m_viewModelEmitter;
    ToolsTreeModel* m_treeModel = nullptr;
    ToolsPanelRegistry* m_panelRegistry = nullptr;
    int m_currentPanelType = 0;     // 0 == ToolPanelType::None
    QString m_currentNodeId;
};
