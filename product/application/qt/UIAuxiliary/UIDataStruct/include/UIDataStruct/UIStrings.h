#pragma once

#include <map>
#include <QtGlobal>
#include <QObject>
#include <QString>
#include <UIDataStruct/UIDataStructExport.h>

namespace UIStrings{
Q_NAMESPACE_EXPORT(UIDataStruct_EXPORT)
void registerMetaObject();

enum class LocalizedString{
    LocalizedString_None,
    LocalizedString_OkButtonLabel,
    LocalizedString_CancelButtonLabel
};
Q_ENUM_NS(LocalizedString)

enum class LocalizedStringWithParam{
    LocalizedStringWithParam_None,
    LocalizedStringWithParam_TestParm
};
Q_ENUM_NS(LocalizedStringWithParam)
}
