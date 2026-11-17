#pragma once

#include <UIEvents/UIEventsExport.h>
#include <UIEventBus/UIEvent.h>

class UIEvents_EXPORT UISettingsEvent final : public UIManager::UIEvent<UISettingsEvent>
{
public:
    enum class Action
    {
        ShowSettings
    };

    static QEvent::Type eventType();

    explicit UISettingsEvent(Action action);

    const Action mAction;
};
