#pragma once

#include <memory>

#include <ucf/services/ChatService/ChatServiceImplExport.h>

namespace ucf::framework { class ICoreFramework; }
namespace ucf::service { class IChatService; }

namespace ucf::service::impl {

CHAT_SERVICE_IMPL_API std::shared_ptr<IChatService> createChatService(std::weak_ptr<framework::ICoreFramework> framework);

} // namespace ucf::service::impl
