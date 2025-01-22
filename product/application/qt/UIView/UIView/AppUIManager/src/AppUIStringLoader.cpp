#include "AppUIStringLoader.h"

#include <QCoreApplication>

#include "LoggerDefine/LoggerDefine.h"

AppUIStringLoader::AppUIStringLoader()
    : mLocalizedString({
        {commonHead::model::LocalizedString::None, ""},
        {commonHead::model::LocalizedString::OkButtonLabel, QT_TR_NOOP("Ok")},
        {commonHead::model::LocalizedString::CancelButtonLabel, QT_TR_NOOP("Cancel")},
    })
    , mLocalizedStringWithParam({
        {commonHead::model::LocalizedStringWithParam::None, ""},
        {commonHead::model::LocalizedStringWithParam::TestParm, QT_TR_NOOP("test param {{0}}, only 1 param")},
    })
{
    UIVIEW_LOG_DEBUG("create string loader");
}

std::string AppUIStringLoader::getLocalizedStringOrigin(commonHead::model::LocalizedString stringId) const
{
    if (auto iter = mLocalizedString.find(stringId); iter != mLocalizedString.end())
    {
        return QCoreApplication::translate("AppUIStringLoader", iter->second.c_str()).toStdString();
    }
    return "Unknown LocalizedString";
}

std::string AppUIStringLoader::getLocalizedStringWithParamsOrigin(commonHead::model::LocalizedStringWithParam stringId) const
{
    if (auto iter = mLocalizedStringWithParam.find(stringId); iter != mLocalizedStringWithParam.end())
    {
        return QCoreApplication::translate("AppUIStringLoader", iter->second.c_str()).toStdString();
    }
    return "Unknown LocalizedStringWithParams";
}
