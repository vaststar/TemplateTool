#pragma once

#include <map>
#include <string>

#include <commonHead/ResourceStringLoader/IResourceStringLoader.h>
#include <ResourceStringMap.h>

class AppUIStringLoader: public commonHead::IResourceStringLoader
{
public:
    AppUIStringLoader();
protected:
    virtual std::string getNonLocalizedStringOrigin(commonHead::model::NonLocalizedString stringId) const override;
    virtual std::string getLocalizedStringOrigin(commonHead::model::LocalizedString stringId) const override;
    virtual std::string getLocalizedStringWithParamsOrigin(commonHead::model::LocalizedStringWithParam stringId) const override;
private:
    const ResourceStringMap::LocalizedStringMaps mLocalizedStringMaps;
};