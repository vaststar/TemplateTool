#pragma once

#include <memory>
#include <vector>

#include <commonhead/utilities/VMNotificationHelper/VMNotificationHelper.h>
#include <commonhead/viewmodels/SettingsViewModel/ISettingsViewModel.h>

namespace commonHead {
class IResourceLoader;
}

namespace commonHead::viewModels {

class SettingsViewModel : public virtual ISettingsViewModel,
                          public virtual commonHead::utilities::VMNotificationHelper<ISettingsViewModelCallback>
{
public:
    explicit SettingsViewModel(commonHead::ICommonHeadFrameworkWptr framework);
    ~SettingsViewModel() override;

    std::string getViewModelName() const override;

    model::SettingsTreePtr getSettingsTree() const override;
    bool isSettingsTreeReady() const override;
    void selectNode(const std::string& nodeId) override;
    void reloadTree() override;

protected:
    void init() override;

private:
    [[nodiscard]] std::shared_ptr<commonHead::IResourceLoader> lockResourceLoader() const;
    [[nodiscard]] bool buildSettingsTree();
    [[nodiscard]] bool refreshTreeNodeData();
    // DFS-collect the full node list (excluding the virtual root), in pre-order.
    std::vector<model::SettingsNodeData> snapshotAllNodes() const;

private:
    model::SettingsTreePtr m_settingsTree;
    bool m_ready = false;
};

} // namespace commonHead::viewModels
