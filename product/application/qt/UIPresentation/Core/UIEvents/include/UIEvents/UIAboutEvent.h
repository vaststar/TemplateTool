#pragma once

#include <UIEvents/UIEventsExport.h>
#include <UIEventBus/UIEvent.h>

class UIEvents_EXPORT UIAboutEvent final : public UIManager::UIEvent<UIAboutEvent>
{
public:
    enum class Action
    {
        ShowAboutDialog
    };

    static QEvent::Type eventType();

    explicit UIAboutEvent(Action action);

    const Action mAction;
};
