#pragma once

#include <commonhead/viewmodels/ChatViewModel/ChatViewModelTypes.h>
#include <ucf/services/ChatService/ChatTypes.h>

namespace commonHead::viewModels::utils {

[[nodiscard]] model::ChatSnapshot toViewModelSnapshot(ucf::service::chat::Snapshot source);

} // namespace commonHead::viewModels::utils
