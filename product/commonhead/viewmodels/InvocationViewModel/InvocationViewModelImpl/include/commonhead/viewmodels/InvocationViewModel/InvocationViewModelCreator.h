#pragma once

#include <memory>

#include <commonhead/viewmodels/InvocationViewModel/IInvocationViewModel.h>
#include <commonhead/viewmodels/InvocationViewModel/InvocationViewModelImplExport.h>

namespace commonHead::viewModels::impl{

INVOCATION_VIEW_MODEL_IMPL_API std::shared_ptr<IInvocationViewModel> createInvocationViewModel(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

}
