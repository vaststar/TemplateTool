#pragma once

#include <memory>

#include <ucf/Infrastructure/InfrastructureCommonFile/InfrastructureExport.h>

namespace ucf::agents {

class IMiniAppRuntimeAgent;

[[nodiscard]] Infrastructure_EXPORT std::shared_ptr<IMiniAppRuntimeAgent> createMiniAppRuntimeAgent();

} // namespace ucf::agents
