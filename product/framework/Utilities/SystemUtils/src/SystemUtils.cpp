#include <ucf/Utilities/SystemUtils/SystemUtils.h>

#if defined(_WIN32)
#include "SystemUtils_Win.h"
#elif defined(__APPLE__)
#include "SystemUtils_Mac.h"
#endif

namespace ucf::utilities {

SystemColorScheme SystemUtils::getSystemColorScheme()
{
#if defined(_WIN32)
    return SystemUtils_Win::getSystemColorScheme();
#elif defined(__APPLE__)
    return SystemUtils_Mac::getSystemColorScheme();
#else
    return SystemColorScheme::Light;
#endif
}

}
