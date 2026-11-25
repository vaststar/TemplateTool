#pragma once

#include <memory>

#include <commonhead/viewmodels/ChatViewModel/ChatViewModelImplExport.h>
#include <commonhead/viewmodels/ChatViewModel/IChatViewModel.h>

namespace commonHead::viewModels::impl {

CHAT_VIEW_MODEL_IMPL_API std::shared_ptr<IChatViewModel>
createChatViewModel(commonHead::ICommonHeadFrameworkWptr framework);

} // namespace commonHead::viewModels::impl
