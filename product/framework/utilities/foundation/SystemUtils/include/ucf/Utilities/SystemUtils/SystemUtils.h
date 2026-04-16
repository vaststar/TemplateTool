#pragma once

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities {

enum class SystemColorScheme { Light, Dark };

class Utilities_EXPORT SystemUtils final
{
public:
    static SystemColorScheme getSystemColorScheme();
};

}
