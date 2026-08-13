#pragma once

#include <concepts>

#include <QEvent>
#include <UIEventBus/UIEventBusExport.h>

namespace UIManager {

/**
 * @brief CRTP base class for type-safe UI events.
 *
 * Usage:
 *   class MyEvent : public UIEvent<MyEvent> { ... };
 *
 * Each concrete event owns an exported eventType() function. The function is
 * implemented by the event module and passes its process-wide type here.
 * The CRTP friend trick prevents misuse like "class A : public UIEvent<B>".
 */
template <typename DerivedEvent>
class UIEvent : public QEvent
{
private:
    explicit UIEvent(QEvent::Type type)
        : QEvent(type)
    {
    }

    friend DerivedEvent;
};

/// A concrete UI event must use the matching CRTP base and expose the
/// process-wide event type owned by its event module.
template <typename EventT>
concept UIEventType =
    std::derived_from<EventT, UIEvent<EventT>>
    && requires {
        { EventT::eventType() } -> std::same_as<QEvent::Type>;
    };

} // namespace UIManager
