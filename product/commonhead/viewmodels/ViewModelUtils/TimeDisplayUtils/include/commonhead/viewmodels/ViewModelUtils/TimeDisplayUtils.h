#pragma once

#include <string>
#include <string_view>

#include <commonhead/viewmodels/ViewModelUtils/ViewModelUtilsExport.h>

namespace commonHead::utilities {

struct VIEW_MODEL_UTILS_API TimeDisplayFormat final
{
    std::string_view localPattern;
    std::string_view utcFallbackPattern;
    std::string_view failureText;
};

// Applies the commonhead display-time policy without exposing TimeUtils types
// to ViewModel consumers: prefer the system timezone, then explicitly use UTC.
class VIEW_MODEL_UTILS_API TimeDisplayUtils final
{
public:
    [[nodiscard]] static std::string formatCurrentUserTime(
        const TimeDisplayFormat& format);

    TimeDisplayUtils() = delete;
    TimeDisplayUtils(const TimeDisplayUtils&) = delete;
    TimeDisplayUtils& operator=(const TimeDisplayUtils&) = delete;
    ~TimeDisplayUtils() = delete;
};

} // namespace commonHead::utilities
