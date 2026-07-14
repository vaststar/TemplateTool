#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

#include <ucf/Infrastructure/WebViewEngine/IRequestInterceptor.h>
#include <ucf/Infrastructure/WebViewEngine/WebViewTypes.h>

namespace ucf::infrastructure::webview {

class InterceptorDispatcher
{
public:
    [[nodiscard]] InterceptorId add(std::shared_ptr<IRequestInterceptor> interceptor);
    void remove(InterceptorId id);
    void clear();

    [[nodiscard]] InterceptResult dispatch(const WebRequest& request) const;

    // Copy the current interceptors (in dispatch order) so callers can run the
    // match/intercept loop off-thread, without holding the dispatcher lock.
    [[nodiscard]] std::vector<std::shared_ptr<IRequestInterceptor>> snapshot() const;

    // Run the match/intercept loop over a snapshot. Static and thread-safe.
    [[nodiscard]] static InterceptResult dispatchSnapshot(
        const std::vector<std::shared_ptr<IRequestInterceptor>>& interceptors,
        const WebRequest& request);

private:
    struct Item
    {
        InterceptorId id = 0;
        int priority = 0;
        std::uint64_t sequence = 0;
        std::shared_ptr<IRequestInterceptor> interceptor;
    };

    void sortItemsLocked();

private:
    mutable std::mutex m_mutex;
    std::vector<Item> m_items;
    std::atomic<InterceptorId> m_nextId { 1 };
    std::uint64_t m_nextSequence = 1;
};

} // namespace ucf::infrastructure::webview
