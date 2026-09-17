#pragma once

#include "../Output.h"

#include <SableLog/Config.h>

#include <string_view>

namespace sablelog::detail {

class ConsoleOutput final : public Output
{
public:
    explicit ConsoleOutput(const ConsoleConfig& config);
    ~ConsoleOutput() override;

    void write(Level level, std::string_view renderedLine) override;
    void flush() override;

private:
    bool mUseColor{};
};

} // namespace sablelog::detail
