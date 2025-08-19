#pragma once

#include <string>
#include <initializer_list>
#include <ResourceString.h>

namespace commonHead{

class IResourceStringLoader
{
public:
    virtual ~IResourceStringLoader() = default;
    std::string getLocalizedString(model::LocalizedString stringId) const;
    std::string getLocalizedStringWithParams(model::LocalizedStringWithParam stringId, const std::initializer_list<std::string>& params) const;
protected:
    virtual std::string getLocalizedStringOrigin(model::LocalizedString stringId) const = 0;
    virtual std::string getLocalizedStringWithParamsOrigin(model::LocalizedStringWithParam stringId) const = 0;
private:
    std::string replaceMarker(const std::string& text, const std::string& marker, const std::string& newText) const;
};
}