#include "InterceptorDispatcher.h"

#include <algorithm>

namespace ucf::infrastructure::webview {

InterceptorId InterceptorDispatcher::add(std::shared_ptr<IRequestInterceptor> interceptor)
{
    if (!interceptor)
    {
        return 0;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    const InterceptorId newId = m_nextId.fetch_add(1);
    Item item;
    item.id = newId;
    item.priority = interceptor->priority();
    item.sequence = m_nextSequence++;
    item.interceptor = std::move(interceptor);
    m_items.push_back(std::move(item));
    sortItemsLocked();
    return newId;
}

void InterceptorDispatcher::remove(InterceptorId id)
{
    if (id == 0)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    m_items.erase(std::remove_if(m_items.begin(), m_items.end(), [id](const Item& item) {
        return item.id == id;
    }), m_items.end());
}

void InterceptorDispatcher::clear()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_items.clear();
}

InterceptResult InterceptorDispatcher::dispatch(const WebRequest& request) const
{
    return dispatchSnapshot(snapshot(), request);
}

std::vector<std::shared_ptr<IRequestInterceptor>> InterceptorDispatcher::snapshot() const
{
    std::vector<std::shared_ptr<IRequestInterceptor>> result;
    std::lock_guard<std::mutex> lock(m_mutex);
    result.reserve(m_items.size());
    for (const Item& item : m_items)
    {
        result.push_back(item.interceptor);
    }
    return result;
}

InterceptResult InterceptorDispatcher::dispatchSnapshot(
    const std::vector<std::shared_ptr<IRequestInterceptor>>& interceptors,
    const WebRequest& request)
{
    for (const auto& interceptor : interceptors)
    {
        if (!interceptor || !interceptor->match(request))
        {
            continue;
        }

        const InterceptResult result = interceptor->intercept(request);
        if (result.action != InterceptAction::Continue)
        {
            return result;
        }
    }

    return {};
}

void InterceptorDispatcher::sortItemsLocked()
{
    std::stable_sort(m_items.begin(), m_items.end(), [](const Item& lhs, const Item& rhs) {
        if (lhs.priority != rhs.priority)
        {
            return lhs.priority > rhs.priority;
        }
        return lhs.sequence < rhs.sequence;
    });
}

} // namespace ucf::infrastructure::webview
