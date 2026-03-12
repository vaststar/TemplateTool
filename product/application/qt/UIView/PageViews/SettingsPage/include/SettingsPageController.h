#pragma once

#include <QObject>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"
#include "SettingsTreeModel.h"
#include "ViewModelSingalEmitter/SettingsViewModelEmitter.h"

namespace commonHead::viewModels {
    class ISettingsViewModel;
}
namespace commonHead::viewModels::model {
    struct SettingsTreeNodeChange;
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
    void onLanguageChanged() override;

signals:
    void treeModelChanged();
    void currentPanelQmlChanged();
    void currentNodeIdChanged();

private slots:
    void onSettingsTreeChanged(const std::shared_ptr<commonHead::viewModels::model::ISettingsTree>& tree);
    void onSettingsTreeStructureChanged(const commonHead::viewModels::model::SettingsTreeNodeChange& change);
    void onSettingsTreeItemsUpdated();
    void onSettingsTreeItemUpdated(const QString& nodeId);
    void onCurrentSettingsNodeChanged(const QString& nodeId, int panelType);

private:
    QString mapPanelTypeToQml(int panelType) const;

    std::shared_ptr<commonHead::viewModels::ISettingsViewModel> m_settingsViewModel;
    std::shared_ptr<UIVMSignalEmitter::SettingsViewModelEmitter> m_viewModelEmitter;
    SettingsTreeModel* m_treeModel = nullptr;
    QString m_currentPanelQml;
    QString m_currentNodeId;
};
