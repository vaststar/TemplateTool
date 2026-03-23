#pragma once

#include <QObject>
#include <QEvent>
#include <UIEventBus/UIEventBusExport.h>
#include <UIEventBus/UIEvent.h>

namespace UIManager {

/**
 * @brief Application-level event bus for cross-controller communication.
 *
 * - Uses QEvent::Type as key (compile-time type safety, no string matching)
 * - Listeners are QObject*; automatically removed on QObject::destroyed
 * - send() dispatches synchronously via QCoreApplication::sendEvent
 * - Must be used from the main thread only
 */
class UIEventBus_EXPORT IUIEventBus : public QObject
{
    Q_OBJECT
public:
    IUIEventBus() = default;
    ~IUIEventBus() override = default;

    /// Create the default UIEventBus implementation.
    static std::unique_ptr<IUIEventBus> createInstance(QObject* parent = nullptr);

    /// Send an event to all registered listeners of the matching type.
    virtual void send(QEvent* event) const = 0;

    /// Type-safe listener registration.
    template <typename DerivedEvent>
    void addListener(QObject* listener)
    {
        addListener(UIEvent<DerivedEvent>::type, listener);
    }

    /// Register listener for multiple event types at once.
    template <typename... Events>
    void addListeners(QObject* listener)
    {
        (addListener<Events>(listener), ...);
    }

public slots:
    virtual void removeListener(QObject* listener) = 0;

protected:
    virtual void addListener(QEvent::Type type, QObject* listener) = 0;

    IUIEventBus(const IUIEventBus&) = delete;
    IUIEventBus& operator=(const IUIEventBus&) = delete;
};

} // namespace UIManager
