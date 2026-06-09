#pragma once

#include <vector>

#include <commonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h>
#include <commonHead/viewModels/SettingsViewModel/ISettingsViewModel.h>

namespace commonHead::viewModels {

class SettingsViewModel : public virtual ISettingsViewModel,
                          public virtual commonHead::utilities::VMNotificationHelper<ISettingsViewModelCallback>
{
public:
    explicit SettingsViewModel(commonHead::ICommonHeadFrameworkWptr framework);
    ~SettingsViewModel() override = default;

    std::string getViewModelName() const override;

    model::SettingsTreePtr getSettingsTree() const override;
    bool isSettingsTreeReady() const override;
    void selectNode(const std::string& nodeId) override;
    void reloadTree() override;

protected:
    void init() override;

private:
    void buildSettingsTree();
    void refreshTreeNodeData();
    // DFS-collect the full node list (excluding the virtual root), in pre-order.
    std::vector<model::SettingsNodeData> snapshotAllNodes() const;

private:
    model::SettingsTreePtr m_settingsTree;
    bool m_ready = false;
};

} // namespace commonHead::viewModels
