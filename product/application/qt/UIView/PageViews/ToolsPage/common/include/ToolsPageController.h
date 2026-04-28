#pragma once

#include <QObject>
#include <QtQml>
#include "UIViewCommon/UIViewBase/include/UIViewController.h"
#include "ToolsTreeModel.h"
#include "ToolsPanelRegistry.h"
#include "UIViewCommon/ViewModelSingalEmitter/ToolsViewModelEmitter.h"

namespace commonHead::viewModels {
    class IToolsViewModel;
}
namespace commonHead::viewModels::model {
    struct ToolsTreeNodeChange;
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
    void onToolsTreeChanged(const std::shared_ptr<commonHead::viewModels::model::IToolsTree>& tree);
    void onToolsTreeStructureChanged(const commonHead::viewModels::model::ToolsTreeNodeChange& change);
    void onToolsTreeItemsUpdated();
    void onToolsTreeItemUpdated(const QString& nodeId);
    void onCurrentToolNodeChanged(const QString& nodeId, int panelType);

private:
    std::shared_ptr<commonHead::viewModels::IToolsViewModel> m_toolsViewModel;
    std::shared_ptr<UIVMSignalEmitter::ToolsViewModelEmitter> m_viewModelEmitter;
    ToolsTreeModel* m_treeModel = nullptr;
    ToolsPanelRegistry* m_panelRegistry = nullptr;
    int m_currentPanelType = 0;
    QString m_currentNodeId;
};
