#pragma once

#include <string>

#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

namespace ucf::utilities{
class Utilities_EXPORT StringUtils final
{
public:
    static std::string trim(const std::string& str);
private:
    static std::string ltrim(const std::string& str);
    static std::string rtrim(const std::string& str);
};
}