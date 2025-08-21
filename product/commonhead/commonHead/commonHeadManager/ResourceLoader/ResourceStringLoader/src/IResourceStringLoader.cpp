#include <commonHead/ResourceStringLoader/IResourceStringLoader.h>

#include <regex>

namespace commonHead{
std::string IResourceStringLoader::getLocalizedString(model::LocalizedString stringId) const
{
    return getLocalizedStringOrigin(stringId);
}

std::string IResourceStringLoader::getLocalizedStringWithParams(model::LocalizedStringWithParam stringId, const std::initializer_list<std::string>& params) const
{
    if (std::string originString = getLocalizedStringWithParamsOrigin(stringId); !originString.empty())
    {
        size_t i = 0;
        for (const auto& param : params)
        {
            originString = replaceMarker(originString, std::to_string(i), param);
            ++i;
        }
        return originString;
    }
    return {};
}

std::string IResourceStringLoader::replaceMarker(const std::string& text, const std::string& marker, const std::string& newText) const
{
    return std::regex_replace(text, std::regex(R"(\{\{)" + marker + R"(\}\})"), newText);
}
}