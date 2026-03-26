#pragma once

#include <string>
#include <initializer_list>
#include <ResourceString.h>
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace commonHead{

class COMMONHEAD_EXPORT IResourceStringLoader
{
public:
    IResourceStringLoader() = default;
    IResourceStringLoader(const IResourceStringLoader&) = delete;
    IResourceStringLoader(IResourceStringLoader&&) = delete;
    IResourceStringLoader& operator=(const IResourceStringLoader&) = delete;
    IResourceStringLoader& operator=(IResourceStringLoader&&) = delete;
    virtual ~IResourceStringLoader() = default;
public:
    std::string getNonLocalizedString(model::NonLocalizedString stringId) const;
    std::string getLocalizedString(model::LocalizedString stringId) const;
    std::string getLocalizedStringWithParams(model::LocalizedStringWithParam stringId, const std::initializer_list<std::string>& params) const;
protected:
    virtual std::string getNonLocalizedStringOrigin(model::NonLocalizedString stringId) const = 0;
    virtual std::string getLocalizedStringOrigin(model::LocalizedString stringId) const = 0;
    virtual std::string getLocalizedStringWithParamsOrigin(model::LocalizedStringWithParam stringId) const = 0;
private:
    std::string replaceMarker(const std::string& text, const std::string& marker, const std::string& newText) const;
};
}