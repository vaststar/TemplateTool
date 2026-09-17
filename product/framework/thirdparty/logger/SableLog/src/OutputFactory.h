#pragma once

#include "Output.h"

#include <SableLog/Config.h>

#include <memory>
#include <vector>

namespace sablelog::detail {

struct OutputEntry final
{
    Level minimumLevel{Level::Off};
    std::unique_ptr<Output> output;
};

class OutputFactory final
{
public:
    OutputFactory() = delete;

    static void normalizeAndValidate(RuntimeConfig& config);
    [[nodiscard]] static std::vector<OutputEntry> create(const LoggerConfig& config);
};

} // namespace sablelog::detail
