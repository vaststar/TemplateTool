#pragma once

#include <UIEventBus/UIEvent.h>

// Broadcast when the screen configuration changes at runtime (DPI/scale,
// resolution, available geometry, screens added/removed).
class UIScreenChangedEvent : public UIManager::UIEvent<UIScreenChangedEvent>
{
public:
    UIScreenChangedEvent();
};
