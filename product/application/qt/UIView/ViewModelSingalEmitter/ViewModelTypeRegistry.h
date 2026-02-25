#pragma once
#include <QMetaType>
#include <commonHead/viewModels/ViewModelDataDefine/VideoFrame.h>

Q_DECLARE_METATYPE(commonHead::viewModels::model::VideoFrame)

namespace UIViewModelTypeRegistry
{
inline void registerTypes()
{
    qRegisterMetaType<commonHead::viewModels::model::VideoFrame>();
}
}