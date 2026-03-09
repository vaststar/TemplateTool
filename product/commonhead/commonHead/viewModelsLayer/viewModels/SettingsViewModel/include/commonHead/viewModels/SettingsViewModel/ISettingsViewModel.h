#pragma once

#include <memory>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>
#include <commonHead/viewModels/SettingsViewModel/ISettingsModel.h>

namespace commonHead::viewModels {

class COMMONHEAD_EXPORT ISettingsViewModel : public IViewModel
{
public:
    using IViewModel::IViewModel;
    ISettingsViewModel(const ISettingsViewModel&) = delete;
    ISettingsViewModel(ISettingsViewModel&&) = delete;
    ISettingsViewModel& operator=(const ISettingsViewModel&) = delete;
    ISettingsViewModel& operator=(ISettingsViewModel&&) = delete;
    virtual ~ISettingsViewModel() = default;

public:
    virtual std::string getViewModelName() const = 0;

    virtual model::SettingsTreePtr getSettingsTree() const = 0;

public:
    static std::shared_ptr<ISettingsViewModel> createInstance(
        commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};

} // namespace commonHead::viewModels
