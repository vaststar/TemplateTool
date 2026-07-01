#pragma once

#include <mutex>
#include <memory>
#include <functional>

namespace ucf::utilities{
// Mirror of NotificationHelper for the reverse direction: a lower layer
// (Manager / Model) holds a single upward sink and forwards events to it.
// NotificationHelper multicasts Service -> external consumers; SinkNotifier
// single-casts lower-layer -> Service. The sink is held as a weak_ptr so a
// released target is skipped instead of keeping it alive.
template <typename TSink>
class SinkNotifier
{
public:
    SinkNotifier() = default;
    SinkNotifier(const SinkNotifier&) = delete;
    SinkNotifier(SinkNotifier&&) = delete;
    SinkNotifier& operator=(const SinkNotifier&) = delete;
    SinkNotifier& operator=(SinkNotifier&&) = delete;
    virtual ~SinkNotifier() = default;
public:
    void setNotificationSink(std::weak_ptr<TSink> sink)
    {
        std::scoped_lock loc(mSinkMutex);
        mSink = std::move(sink);
    }

protected:
    // Invoke a sink method when the sink is still alive; skip silently otherwise.
    template <typename Method, typename... Args>
    void notifySink(Method method, Args&&... args)
    {
        std::weak_ptr<TSink> sinkCopy;
        {
            std::scoped_lock loc(mSinkMutex);
            sinkCopy = mSink;
        }
        if (auto sink = sinkCopy.lock())
        {
            std::invoke(method, sink, std::forward<Args>(args)...);
        }
    }

    // Exposes the current sink so an intermediate layer can forward it downward.
    std::weak_ptr<TSink> currentSink() const
    {
        std::scoped_lock loc(mSinkMutex);
        return mSink;
    }

private:
    mutable std::mutex mSinkMutex;
    std::weak_ptr<TSink> mSink;
};
}
