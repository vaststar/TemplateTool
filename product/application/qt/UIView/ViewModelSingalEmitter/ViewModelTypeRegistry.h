#pragma once
#include <QMetaType>
#include <commonHead/viewModels/ViewModelDataDefine/Image.h>

Q_DECLARE_METATYPE(commonHead::viewModels::model::Image)

namespace UIViewModelTypeRegistry
{
inline void registerTypes()
{
    qRegisterMetaType<commonHead::viewModels::model::Image>();
}
}