#pragma once

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
    void selectNode(const std::string& nodeId) override;
    void reloadTree() override;

protected:
    void init() override;

private:
    void buildSettingsTree();
    void refreshTreeNodeData();

private:
    model::SettingsTreePtr m_settingsTree;
};

} // namespace commonHead::viewModels
