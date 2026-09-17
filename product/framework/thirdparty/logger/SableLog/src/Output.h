#pragma once

#include <SableLog/Config.h>

#include <string_view>

namespace sablelog::detail {

class Output
{
public:
    virtual ~Output();

    Output(const Output&) = delete;
    Output& operator=(const Output&) = delete;
    Output(Output&&) = delete;
    Output& operator=(Output&&) = delete;

    virtual void write(Level level, std::string_view renderedLine) = 0;
    virtual void flush() = 0;

protected:
    Output() noexcept;
};

} // namespace sablelog::detail
