#pragma once

#include <UIEvents/UIEventsExport.h>
#include <UIEventBus/UIEvent.h>

class UIEvents_EXPORT UIUpgradeEvent final : public UIManager::UIEvent<UIUpgradeEvent>
{
public:
    enum class Action
    {
        CheckForUpgrade
    };

    static QEvent::Type eventType();

    explicit UIUpgradeEvent(Action action);

    const Action mAction;
};
