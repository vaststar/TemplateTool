#pragma once

#include <vector>

#include <QMetaType>

#include <UIViewModelSignalBridge/metatypes/CommonMetaTypes.h>
#include <commonhead/viewmodels/SettingsViewModel/ISettingsModel.h>

Q_DECLARE_METATYPE(commonHead::viewModels::model::SettingsTreePtr)
Q_DECLARE_METATYPE(commonHead::viewModels::model::SettingsNodeData)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::SettingsNodeData>)
