#include "AppUIStringLoader.h"

#include <QCoreApplication>

#include "LoggerDefine/LoggerDefine.h"


namespace CPP_LocalizedString{
inline static constexpr auto OkButtonLabel = QT_TR_NOOP("Ok");
inline static constexpr auto CancelButtonLabel = QT_TR_NOOP("Cancel");
}

namespace CPP_LocalizedStringWithParam{
inline static constexpr auto TestParm = QT_TR_NOOP("test param {{0}}, only 1 param");
}



AppUIStringLoader::AppUIStringLoader()
    : mLocalizedString({
        {commonHead::model::LocalizedString::None, ""},
        {commonHead::model::LocalizedString::OkButtonLabel, CPP_LocalizedString::OkButtonLabel},
        {commonHead::model::LocalizedString::CancelButtonLabel, CPP_LocalizedString::CancelButtonLabel},
    })
    , mLocalizedStringWithParam({
        {commonHead::model::LocalizedStringWithParam::None, ""},
        {commonHead::model::LocalizedStringWithParam::TestParm, CPP_LocalizedStringWithParam::TestParm},
    })
{
    UIVIEW_LOG_DEBUG("create string loader");
}

std::string AppUIStringLoader::getLocalizedStringOrigin(commonHead::model::LocalizedString stringId) const
{
    if (auto iter = mLocalizedString.find(stringId); iter != mLocalizedString.end())
    {
        return QCoreApplication::translate("CPP_LocalizedString", iter->second.c_str()).toStdString();
    }
    return "Unknown LocalizedString";
}

std::string AppUIStringLoader::getLocalizedStringWithParamsOrigin(commonHead::model::LocalizedStringWithParam stringId) const
{
    if (auto iter = mLocalizedStringWithParam.find(stringId); iter != mLocalizedStringWithParam.end())
    {
        return QCoreApplication::translate("CPP_LocalizedStringWithParam", iter->second.c_str()).toStdString();
    }
    return "Unknown LocalizedStringWithParams";
}
