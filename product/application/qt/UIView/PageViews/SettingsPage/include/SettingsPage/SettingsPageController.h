#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <QObject>
#include <QtQml>
#include "UIViewBase/UIViewController.h"
#include "SettingsTreeModel.h"
#include "ViewModelSingalEmitter/SettingsViewModelEmitter.h"

namespace commonHead::viewModels {
    class ISettingsViewModel;
}
namespace commonHead::viewModels::model {
    class ISettingsTree;
    struct SettingsNodeData;
    enum class SettingsPanelType : uint8_t;
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
    void onSettingsTreeReady();
    void onSettingsNodesAdded(const std::vector<commonHead::viewModels::model::SettingsNodeData>& nodes);
    void onSettingsNodesUpdated(const std::vector<commonHead::viewModels::model::SettingsNodeData>& nodes);
    void onSettingsNodesRemoved(const std::vector<std::string>& nodeIds);

private:
    QString mapPanelTypeToQml(commonHead::viewModels::model::SettingsPanelType panelType) const;
    commonHead::viewModels::model::SettingsPanelType panelTypeOf(const std::string& nodeId) const;
    std::string findFirstSelectableNodeId() const;
    void ensureValidSelection();

    std::shared_ptr<commonHead::viewModels::ISettingsViewModel> m_settingsViewModel;
    std::shared_ptr<UIVMSignalEmitter::SettingsViewModelEmitter> m_viewModelEmitter;
    SettingsTreeModel* m_treeModel = nullptr;
    QString m_currentPanelQml;
    QString m_currentNodeId;
};
