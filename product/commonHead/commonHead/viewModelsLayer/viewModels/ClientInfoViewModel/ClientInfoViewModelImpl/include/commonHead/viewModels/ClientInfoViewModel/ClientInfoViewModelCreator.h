#pragma once

#include <memory>

#include <commonHead/viewModels/ClientInfoViewModel/IClientInfoViewModel.h>
#include <commonHead/viewModels/ClientInfoViewModel/ClientInfoViewModelImplExport.h>

namespace commonHead::viewModels::impl{

CLIENT_INFO_VIEW_MODEL_IMPL_API std::shared_ptr<IClientInfoViewModel> createClientInfoViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

}
