#pragma once

#include <UIView/UIViewExport.h>

namespace UIViewModule
{
// Establishes an explicit runtime dependency on the UIView QML module.
UIView_EXPORT void ensureLoaded();
}
