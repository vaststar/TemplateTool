#pragma once

#include <memory>

#include <commonHead/viewModels/SettingsViewModel/ISettingsViewModel.h>
#include <commonHead/viewModels/SettingsViewModel/SettingsViewModelImplExport.h>

namespace commonHead::viewModels::impl {

SETTINGS_VIEW_MODEL_IMPL_API std::shared_ptr<ISettingsViewModel> createSettingsViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
