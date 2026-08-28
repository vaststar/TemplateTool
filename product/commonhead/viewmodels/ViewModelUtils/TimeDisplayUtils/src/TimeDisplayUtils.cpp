#include <commonhead/viewmodels/ViewModelUtils/TimeDisplayUtils.h>

#include <ucf/utilities/TimeUtils/TimeUtils.h>

#include <utility>

namespace commonHead::utilities {

std::string TimeDisplayUtils::formatCurrentUserTime(
    const TimeDisplayFormat& format)
{
    const auto instant = ucf::utilities::TimeUtils::now();

    auto systemTimeZone = ucf::utilities::TimeUtils::systemTimeZone();
    if (systemTimeZone)
    {
        auto formatted = ucf::utilities::TimeUtils::format(
            instant,
            systemTimeZone.value(),
            format.localPattern);
        if (formatted)
        {
            return std::move(formatted).value();
        }
    }

    auto utcFormatted = ucf::utilities::TimeUtils::format(
        instant,
        ucf::utilities::TimeZone::utc(),
        format.utcFallbackPattern);

    return utcFormatted
        ? std::move(utcFormatted).value()
        : std::string{format.failureText};
}

} // namespace commonHead::utilities
