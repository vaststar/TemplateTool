#pragma once

#include <string>

#include <ucf/Utilities/UUIDUtils/UUIDUtilsExport.h>

namespace ucf::utilities{
class UUID_UTILS_API UUIDUtils final
{
public:
    static std::string generateUUID();
    static bool isValidUUID(const std::string& str);
private:
    static unsigned char random_char();
    static std::string generate_hex(unsigned int len);
};
}