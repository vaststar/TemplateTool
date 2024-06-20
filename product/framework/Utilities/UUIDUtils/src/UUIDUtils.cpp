#include <set>
#include <array>
#include <vector>
#include <ranges>
#include <sstream>
#include <numeric>
#include <random>
#include <climits>
#include <algorithm>
#include <functional>
#include <string>
#include <cctype>
#include <ucf/Utilities/UUIDUtils/UUIDUtils.h>


namespace ucf::utilities{

unsigned char UUIDUtils::random_char() 
{
    std::random_device rd;
    std::mt19937_64 gen(rd()); 
    std::uniform_int_distribution<> dis(0, 255);
    return static_cast<unsigned char>(dis(gen));
}

std::string UUIDUtils::generate_hex(unsigned int len) 
{
    std::stringstream ss;
    for(auto i = 0; i < len; i++) {
        auto rc = random_char();
        std::stringstream hexstream;
        hexstream << std::hex << int(rc);
        auto hex = hexstream.str(); 
        ss << (hex.length() < 2 ? '0' + hex : hex);
    }        
    return ss.str();
}

std::string UUIDUtils::generateUUID()
{
    std::vector<int> numArray{4, 2, 2, 2, 6};
    std::vector<std::string> hexArray;
    std::transform(numArray.begin(), numArray.end(), std::back_inserter(hexArray), [](int num){return UUIDUtils::generate_hex(num);});
    return std::accumulate(std::next(hexArray.begin()), hexArray.end(), hexArray.front(), [](std::string a, std::string b){return a+"-"+b;});
}

bool UUIDUtils::isValidUUID(const std::string& str)
{
    if (str.length() != 36)
    {
        return false;
    }
    const std::set<int> arrayHappen{8, 13, 18, 23};
    for(size_t i = 0; i < str.length(); ++i)
    {
        if (arrayHappen.contains(i))
        {
            if ('-' != str[i])
            {
                return false;
            }
        }
        else if (std::isxdigit(str[i]))
        {
            return false;
        }
    }
    return true;
}
}