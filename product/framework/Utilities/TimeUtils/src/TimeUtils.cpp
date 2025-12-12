#include <chrono>

#include <ucf/Utilities/TimeUtils/TimeUtils.h>

namespace ucf::utilities {
std::string TimeUtils::getUTCCurrentTime()
{
    return "";
}

time_t TimeUtils::getUTCNowInMilliseconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string TimeUtils::getLocalTimeZone()
{
    return {};//std::string{std::chrono::current_zone()->name()};
}
}