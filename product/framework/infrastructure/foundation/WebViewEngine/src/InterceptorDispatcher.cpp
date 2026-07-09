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
    std::vector<Item> snapshot;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        snapshot = m_items;
    }

    for (const Item& item : snapshot)
    {
        if (!item.interceptor || !item.interceptor->match(request))
        {
            continue;
        }

        const InterceptResult result = item.interceptor->intercept(request);
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
