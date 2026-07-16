#include "WebViewBase.h"

namespace ucf::infrastructure::webview {

InterceptorId WebViewBase::addRequestInterceptor(std::shared_ptr<IRequestInterceptor> interceptor)
{
    return m_dispatcher.add(std::move(interceptor));
}

void WebViewBase::removeRequestInterceptor(InterceptorId id)
{
    m_dispatcher.remove(id);
}

void WebViewBase::clearRequestInterceptors()
{
    m_dispatcher.clear();
}

} // namespace ucf::infrastructure::webview
