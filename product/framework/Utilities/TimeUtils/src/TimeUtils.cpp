#include <chrono>
#include <ctime>

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
#if defined(_WIN32)
    // Windows MSVC supports C++20 timezone library
    return std::string{std::chrono::current_zone()->name()};
#else
    // macOS/Linux: libc++ doesn't support current_zone(), use POSIX API
    std::time_t now = std::time(nullptr);
    std::tm localTm{};
    localtime_r(&now, &localTm);
    return localTm.tm_zone ? std::string(localTm.tm_zone) : "UTC";
#endif
}
}
