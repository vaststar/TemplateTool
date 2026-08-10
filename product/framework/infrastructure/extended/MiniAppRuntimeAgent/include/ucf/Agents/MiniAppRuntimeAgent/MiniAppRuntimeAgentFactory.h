#pragma once

#include <memory>

#include <ucf/Agents/MiniAppRuntimeAgent/MiniAppRuntimeAgentExport.h>

namespace ucf::agents {

class IMiniAppRuntimeAgent;

[[nodiscard]] MINI_APP_RUNTIME_AGENT_API std::shared_ptr<IMiniAppRuntimeAgent> createMiniAppRuntimeAgent();

} // namespace ucf::agents
