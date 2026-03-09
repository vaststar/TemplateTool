#pragma once

#include <commonHead/viewModels/SettingsViewModel/ISettingsViewModel.h>

namespace commonHead::viewModels {

class SettingsViewModel : public ISettingsViewModel
{
public:
    explicit SettingsViewModel(commonHead::ICommonHeadFrameworkWptr framework);
    ~SettingsViewModel() override = default;

    std::string getViewModelName() const override;

    model::SettingsTreePtr getSettingsTree() const override;

protected:
    void init() override;

private:
    void buildSettingsTree();

private:
    model::SettingsTreePtr m_settingsTree;
};

} // namespace commonHead::viewModels
