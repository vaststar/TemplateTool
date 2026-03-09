#pragma once

#include <QObject>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"
#include "ToolsTreeModel.h"

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

signals:
    void treeModelChanged();
    void currentPanelQmlChanged();
    void currentNodeIdChanged();

private:
    QString mapPanelTypeToQml(int panelType) const;
    void selectFirstNode();

    std::shared_ptr<commonHead::viewModels::IToolsViewModel> m_toolsViewModel;
    ToolsTreeModel* m_treeModel = nullptr;
    QString m_currentPanelQml;
    QString m_currentNodeId;
};
