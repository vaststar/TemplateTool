#pragma once

#include <cstdint>
#include <string>

namespace commonHead::viewModels::model {

enum class TrayActionId : std::int32_t
{
    Unknown = 0,
    Quit,
    ShowMainWindow,
    OpenSettings,
    CheckForUpdates,
    ShowAbout
};

enum class TrayMenuItemKind : std::int32_t
{
    Action = 0,
    Separator
};

struct TrayMenuItemData
{
    TrayActionId actionId = TrayActionId::Unknown;
    std::string title;
    TrayMenuItemKind kind = TrayMenuItemKind::Action;
    bool enabled = true;
};

} // namespace commonHead::viewModels::model
