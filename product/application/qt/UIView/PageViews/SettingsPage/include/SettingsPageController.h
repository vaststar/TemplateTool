#pragma once

#include <QObject>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"
#include "SettingsTreeModel.h"

namespace commonHead::viewModels {
    class ISettingsViewModel;
}

class SettingsPageController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(SettingsTreeModel* treeModel READ getTreeModel NOTIFY treeModelChanged)
    Q_PROPERTY(QString currentPanelQml READ getCurrentPanelQml NOTIFY currentPanelQmlChanged)
    Q_PROPERTY(QString currentNodeId READ getCurrentNodeId NOTIFY currentNodeIdChanged)

public:
    explicit SettingsPageController(QObject* parent = nullptr);

    SettingsTreeModel* getTreeModel() const;
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

    std::shared_ptr<commonHead::viewModels::ISettingsViewModel> m_settingsViewModel;
    SettingsTreeModel* m_treeModel = nullptr;
    QString m_currentPanelQml;
    QString m_currentNodeId;
};
