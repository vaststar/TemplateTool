#pragma once

#include <QEvent>
#include <UIEventBus/UIEventBusExport.h>

namespace UIManager {

/**
 * @brief CRTP base class for type-safe UI events.
 *
 * Usage:
 *   class MyEvent : public UIEvent<MyEvent> { ... };
 *
 * Each derived class automatically gets a unique QEvent::Type via
 * QEvent::registerEventType(). The CRTP friend trick prevents misuse
 * like "class A : public UIEvent<B>".
 */
template <typename DerivedEvent>
class UIEvent : public QEvent
{
public:
    inline static const QEvent::Type type{
        static_cast<QEvent::Type>(QEvent::registerEventType())
    };

private:
    UIEvent() : QEvent(type) {}
    friend DerivedEvent;
};

} // namespace UIManager
