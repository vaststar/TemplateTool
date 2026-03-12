#pragma once

#include <QObject>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"
#include "ToolsTreeModel.h"
#include "ViewModelSingalEmitter/ToolsViewModelEmitter.h"

namespace commonHead::viewModels {
    class IToolsViewModel;
}

class ToolsPageController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(ToolsTreeModel* treeModel READ getTreeModel NOTIFY treeModelChanged)
    Q_PROPERTY(QString currentPanelQml READ getCurrentPanelQml NOTIFY currentPanelQmlChanged)
    Q_PROPERTY(QString currentNodeId READ getCurrentNodeId NOTIFY currentNodeIdChanged)

public:
    explicit ToolsPageController(QObject* parent = nullptr);

    ToolsTreeModel* getTreeModel() const;
    QString getCurrentPanelQml() const;
    QString getCurrentNodeId() const;

    Q_INVOKABLE void selectNode(const QString& nodeId);

protected:
    void init() override;
    void onLanguageChanged() override;

signals:
    void treeModelChanged();
    void currentPanelQmlChanged();
    void currentNodeIdChanged();

private slots:
    void onToolsTreeChanged(const std::shared_ptr<commonHead::viewModels::model::IToolsTree>& tree);
    void onCurrentToolNodeChanged(const QString& nodeId, int panelType);

private:
    QString mapPanelTypeToQml(int panelType) const;

    std::shared_ptr<commonHead::viewModels::IToolsViewModel> m_toolsViewModel;
    std::shared_ptr<UIVMSignalEmitter::ToolsViewModelEmitter> m_viewModelEmitter;
    ToolsTreeModel* m_treeModel = nullptr;
    QString m_currentPanelQml;
    QString m_currentNodeId;
};
