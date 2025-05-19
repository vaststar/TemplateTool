#pragma once

#include <map>
#include <string>

#include <commonHead/ResourceLoader/IResourceStringLoader.h>

class AppUIStringLoader: public commonHead::IResourceStringLoader
{
public:
    AppUIStringLoader();
protected:
    virtual std::string getLocalizedStringOrigin(commonHead::model::LocalizedString stringId) const override;
    virtual std::string getLocalizedStringWithParamsOrigin(commonHead::model::LocalizedStringWithParam stringId) const override;
private:
    const std::map<commonHead::model::LocalizedString, std::string> mLocalizedString;
    const std::map<commonHead::model::LocalizedStringWithParam, std::string> mLocalizedStringWithParam;
};