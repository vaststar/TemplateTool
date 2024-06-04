#pragma once

#include <string>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities{
class Utilities_EXPORT TimeUtils final
{
public:
    static std::string getUTCCurrentTime();
    static time_t getUTCNowInMilliseconds();
    static std::string getLocalTimeZone();
public:
    TimeUtils() = delete;
    TimeUtils(const TimeUtils &rhs) = delete;
    TimeUtils& operator=(const TimeUtils &rhs) = delete;
    TimeUtils(TimeUtils &&rhs) = delete;
    TimeUtils& operator=(TimeUtils &&rhs) = delete;
    ~TimeUtils() = delete;
};
}