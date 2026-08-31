#include "AppUIStringLoader.h"

#include <QCoreApplication>

#include "LoggerDefine.h"

AppUIStringLoader::AppUIStringLoader()
{
    UIResourceStringLoader_LOG_DEBUG(
        "AppUIStringLoader constructing, address: " << this);

    UIResourceStringLoader_LOG_DEBUG(
        "AppUIStringLoader constructed, address: " << this);
}

AppUIStringLoader::~AppUIStringLoader()
{
    UIResourceStringLoader_LOG_DEBUG(
        "AppUIStringLoader destroying, address: " << this);

    UIResourceStringLoader_LOG_DEBUG(
        "AppUIStringLoader destructor body finished, address: " << this);
}

std::string AppUIStringLoader::getNonLocalizedStringOrigin(commonHead::model::NonLocalizedString stringId) const
{
    if (auto iter = mLocalizedStringMaps.nonLocalizedStrings.find(stringId); iter != mLocalizedStringMaps.nonLocalizedStrings.end())
    {
        return iter->second;
    }
    return "Unknown NonLocalizedString";
}

std::string AppUIStringLoader::getLocalizedStringOrigin(commonHead::model::LocalizedString stringId) const
{
    if (auto iter = mLocalizedStringMaps.localizedStrings.find(stringId); iter != mLocalizedStringMaps.localizedStrings.end())
    {
        return QCoreApplication::translate("QObject", iter->second.c_str()).toStdString();
    }
    return "Unknown LocalizedString";
}

std::string AppUIStringLoader::getLocalizedStringWithParamsOrigin(commonHead::model::LocalizedStringWithParam stringId) const
{
    if (auto iter = mLocalizedStringMaps.localizedStringWithParams.find(stringId); iter != mLocalizedStringMaps.localizedStringWithParams.end())
    {
        return QCoreApplication::translate("QObject", iter->second.c_str()).toStdString();
    }
    return "Unknown LocalizedStringWithParams";
}
