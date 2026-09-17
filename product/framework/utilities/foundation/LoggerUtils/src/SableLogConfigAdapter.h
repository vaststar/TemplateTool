#pragma once

#include <SableLog/Config.h>

#include <ucf/utilities/LoggerUtils/Config.h>

namespace ucf::utilities::detail {

[[nodiscard]] sablelog::RuntimeConfig toSableLogConfig(LoggingConfig config);

} // namespace ucf::utilities::detail
