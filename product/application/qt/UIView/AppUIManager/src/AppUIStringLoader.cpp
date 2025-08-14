#include "AppUIStringLoader.h"

#include <QCoreApplication>

#include "LoggerDefine/LoggerDefine.h"


// namespace CPP_LocalizedString{
// inline static constexpr auto OkButtonLabel = QT_TR_NOOP("Ok");
// inline static constexpr auto CancelButtonLabel = QT_TR_NOOP("Cancel");
// }

// namespace CPP_LocalizedStringWithParam{
// inline static constexpr auto TestParm = QT_TR_NOOP("test param {{0}}, only 1 param");
// }



AppUIStringLoader::AppUIStringLoader()
    // : mLocalizedString({
    //     {commonHead::model::LocalizedString::None, ""},
    //     {commonHead::model::LocalizedString::OkButtonLabel, CPP_LocalizedString::OkButtonLabel},
    //     {commonHead::model::LocalizedString::CancelButtonLabel, CPP_LocalizedString::CancelButtonLabel},
    // })
    // , mLocalizedStringWithParam({
    //     {commonHead::model::LocalizedStringWithParam::None, ""},
    //     {commonHead::model::LocalizedStringWithParam::TestParm, CPP_LocalizedStringWithParam::TestParm},
    // })
{
    UIVIEW_LOG_DEBUG("create string loader");
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
