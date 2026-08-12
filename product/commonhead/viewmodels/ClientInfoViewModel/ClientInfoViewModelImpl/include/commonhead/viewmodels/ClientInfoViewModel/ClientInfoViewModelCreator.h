#pragma once

#include <memory>

#include <commonhead/viewmodels/ClientInfoViewModel/IClientInfoViewModel.h>
#include <commonhead/viewmodels/ClientInfoViewModel/ClientInfoViewModelImplExport.h>

namespace commonHead::viewModels::impl{

CLIENT_INFO_VIEW_MODEL_IMPL_API std::shared_ptr<IClientInfoViewModel> createClientInfoViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

}
