#include "UIEventBus.h"

#include <QCoreApplication>
#include <QThread>
#include <algorithm>

namespace UIManager {

std::unique_ptr<IUIEventBus> IUIEventBus::createInstance(QObject* parent)
{
    return std::make_unique<UIEventBus>(parent);
}


UIEventBus::UIEventBus(QObject* parent)
    : IUIEventBus()
{
    setParent(parent);
}

void UIEventBus::send(QEvent* event) const
{
    Q_ASSERT_X(QThread::currentThread() == thread(),
               "UIEventBus::send",
               "UIEventBus must be used from the main thread");

    if (!event || m_listenerPairs.empty())
        return;

    // Snapshot: safe against re-entrance and mid-iteration removal/addition
    const auto snapshot = m_listenerPairs;

    for (const auto& [eventType, listener] : snapshot)
    {
        // QPointer becomes null if listener was destroyed during iteration
        if (event->type() == eventType && !listener.isNull())
        {
            QCoreApplication::sendEvent(listener.data(), event);
        }
    }
}

void UIEventBus::addListener(QEvent::Type type, QObject* listener)
{
    Q_ASSERT_X(QThread::currentThread() == thread(),
               "UIEventBus::addListener",
               "UIEventBus must be used from the main thread");

    if (!listener || type < QEvent::User)
        return;

    // Check for duplicate
    for (const auto& [t, l] : m_listenerPairs)
    {
        if (t == type && l == listener)
            return;
    }

    m_listenerPairs.emplace_back(type, listener);

    // Auto-remove when listener is destroyed
    QObject::connect(listener, &QObject::destroyed,
                     this, &UIEventBus::removeListener);
}

void UIEventBus::removeListener(QObject* listener)
{
    Q_ASSERT_X(QThread::currentThread() == thread(),
               "UIEventBus::removeListener",
               "UIEventBus must be used from the main thread");

    if (!listener || m_listenerPairs.empty())
        return;

    auto it = std::remove_if(m_listenerPairs.begin(), m_listenerPairs.end(),
        [listener](const auto& pair) { return pair.second == listener || pair.second.isNull(); });
    m_listenerPairs.erase(it, m_listenerPairs.end());
}

} // namespace UIManager
