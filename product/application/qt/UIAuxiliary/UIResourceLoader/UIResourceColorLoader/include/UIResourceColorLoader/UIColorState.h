#pragma once

#include <QObject>
#include <QQmlEngine>
#include <UIResourceColorLoader/UIResourceColorLoaderExport.h>

namespace UIColorState{
Q_NAMESPACE_EXPORT(UIResourceColorLoader_EXPORT)

enum class ColorState{
    Normal,
    Hovered,
    Pressed,
    Disabled,
    Focused,
    Checked
};
Q_ENUM_NS(ColorState)
}