#pragma once

#include <memory>

#include <commonhead/viewmodels/SettingsViewModel/ISettingsViewModel.h>
#include <commonhead/viewmodels/SettingsViewModel/SettingsViewModelImplExport.h>

namespace commonHead::viewModels::impl {

SETTINGS_VIEW_MODEL_IMPL_API std::shared_ptr<ISettingsViewModel> createSettingsViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
