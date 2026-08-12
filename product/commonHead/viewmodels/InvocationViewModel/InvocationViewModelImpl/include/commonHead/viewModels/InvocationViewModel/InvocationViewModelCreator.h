#pragma once

#include <memory>

#include <commonHead/viewModels/InvocationViewModel/IInvocationViewModel.h>
#include <commonHead/viewModels/InvocationViewModel/InvocationViewModelImplExport.h>

namespace commonHead::viewModels::impl{

INVOCATION_VIEW_MODEL_IMPL_API std::shared_ptr<IInvocationViewModel> createInvocationViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

}
