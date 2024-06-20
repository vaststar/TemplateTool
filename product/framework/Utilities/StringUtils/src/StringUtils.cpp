
#include <ucf/Utilities/StringUtils/StringUtils.h>
#include <algorithm>
#include <vector>
#include <string>
#include <cctype>

namespace ucf::utilities{

std::string StringUtils::trim(const std::string& str)
{
    return ltrim(rtrim(str));
}

std::string StringUtils::ltrim(const std::string& str)
{
    auto res = str;
    res.erase(std::find_if(res.begin(), res.end(), [](char ch) {return !std::isspace(ch);}), res.end());
    return res;
}

std::string StringUtils::rtrim(const std::string& str)
{
    auto res = str;
    res.erase(std::find_if(res.rbegin(), res.rend(), [](char ch) {return !std::isspace(ch);}).base(), res.end());
    return res;
}
}