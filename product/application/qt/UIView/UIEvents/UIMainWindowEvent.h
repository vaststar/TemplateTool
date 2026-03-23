#pragma once

#include <UIEventBus/UIEvent.h>

class UIMainWindowEvent : public UIManager::UIEvent<UIMainWindowEvent>
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

    explicit UIMainWindowEvent(Action action);

    const Action mAction;
};
