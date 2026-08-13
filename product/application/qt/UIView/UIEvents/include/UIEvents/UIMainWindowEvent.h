#pragma once

#include <UIEvents/UIEventsExport.h>
#include <UIEventBus/UIEvent.h>

class UIEvents_EXPORT UIMainWindowEvent final : public UIManager::UIEvent<UIMainWindowEvent>
{
public:
    enum class Action
    {
        Activate,       // Bring window to front
        Minimize,       // Minimize to taskbar/dock
        Maximize,       // Maximize window
        Restore,        // Restore from minimized/maximized
        Hide,           // Hide window
        Show,           // Show window
        Close           // Request close
    };

    static QEvent::Type eventType();

    explicit UIMainWindowEvent(Action action);

    const Action mAction;
};
