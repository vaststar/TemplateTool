#pragma once

#include <memory>

#include <commonhead/viewmodels/NetworkProxyViewModel/INetworkProxyViewModel.h>
#include <commonhead/viewmodels/NetworkProxyViewModel/NetworkProxyViewModelImplExport.h>

namespace commonHead::viewModels::impl {

NETWORK_PROXY_VIEW_MODEL_IMPL_API std::shared_ptr<INetworkProxyViewModel>
createNetworkProxyViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework);

} // namespace commonHead::viewModels::impl
