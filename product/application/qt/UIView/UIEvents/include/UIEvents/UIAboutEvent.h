#pragma once

#include <UIEventBus/UIEvent.h>

class UIAboutEvent : public UIManager::UIEvent<UIAboutEvent>
{
public:
    enum class Action
    {
        ShowAboutDialog
    };

    explicit UIAboutEvent(Action action);

    const Action mAction;
};
