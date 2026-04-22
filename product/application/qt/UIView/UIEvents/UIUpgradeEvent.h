#pragma once

#include <UIEventBus/UIEvent.h>

class UIUpgradeEvent : public UIManager::UIEvent<UIUpgradeEvent>
{
public:
    enum class Action
    {
        CheckForUpgrade
    };

    explicit UIUpgradeEvent(Action action);

    const Action mAction;
};
