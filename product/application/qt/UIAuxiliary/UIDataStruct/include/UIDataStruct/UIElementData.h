#pragma once

#include <QObject>
#include <UIDataStruct/UIDataStructExport.h>

namespace UIElementData{
Q_NAMESPACE_EXPORT(UIDataStruct_EXPORT)
void registerMetaObject();

enum class UIThemeType{
    UIThemeType_SystemDefault,
    UIThemeType_Dark,
    UIThemeType_Light
};
Q_ENUM_NS(UIThemeType)

}