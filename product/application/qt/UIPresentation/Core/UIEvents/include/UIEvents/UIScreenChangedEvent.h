#pragma once

#include <UIEvents/UIEventsExport.h>
#include <UIEventBus/UIEvent.h>

// Broadcast when the screen configuration changes at runtime (DPI/scale,
// resolution, available geometry, screens added/removed).
class UIEvents_EXPORT UIScreenChangedEvent final : public UIManager::UIEvent<UIScreenChangedEvent>
{
public:
    static QEvent::Type eventType();

    UIScreenChangedEvent();
};
