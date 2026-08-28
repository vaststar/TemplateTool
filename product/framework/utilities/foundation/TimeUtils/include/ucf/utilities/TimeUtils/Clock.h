#pragma once

#include <ucf/utilities/TimeUtils/Instant.h>
#include <ucf/utilities/TimeUtils/TimeUtilsExport.h>

namespace ucf::utilities {

class TIME_UTILS_API IClock
{
public:
    IClock(const IClock&) = delete;
    IClock& operator=(const IClock&) = delete;
    virtual ~IClock() = default;

    [[nodiscard]] virtual Instant now() const noexcept = 0;

protected:
    IClock() = default;
};

class TIME_UTILS_API SystemClock final : public IClock
{
public:
    [[nodiscard]] static const SystemClock& instance() noexcept;
    [[nodiscard]] Instant now() const noexcept override;

private:
    SystemClock() = default;
};

} // namespace ucf::utilities
