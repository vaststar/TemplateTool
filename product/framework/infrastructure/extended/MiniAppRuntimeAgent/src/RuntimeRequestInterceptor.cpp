#include "RuntimeRequestInterceptor.h"

#include <string>
#include <utility>

#include "RuntimeResourceResolver.h"

namespace ucf::agents {
namespace {

ucf::infrastructure::webview::InterceptResult makeFail(int code, const std::string& message)
{
    ucf::infrastructure::webview::InterceptResult result;
    result.action = ucf::infrastructure::webview::InterceptAction::Fail;
    result.error = ucf::infrastructure::webview::WebError{ code, message };
    return result;
}

ucf::infrastructure::webview::InterceptResult makeRespond(const ResolvedResource& resource)
{
    ucf::infrastructure::webview::InterceptResult result;
    result.action = ucf::infrastructure::webview::InterceptAction::Respond;
    ucf::infrastructure::webview::WebResponse response;
    response.statusCode = resource.statusCode;
    response.mimeType = resource.mimeType;
    response.headers = resource.headers;
    response.body = resource.body;
    result.response = std::move(response);
    return result;
}

} // namespace

RuntimeRequestInterceptor::RuntimeRequestInterceptor(std::shared_ptr<RuntimeResourceResolver> resolver)
    : m_resolver(std::move(resolver))
{
}

int RuntimeRequestInterceptor::priority() const
{
    return 200;
}

bool RuntimeRequestInterceptor::match(const ucf::infrastructure::webview::WebRequest& request) const
{
    return m_resolver && m_resolver->canHandle(request.url);
}

ucf::infrastructure::webview::InterceptResult RuntimeRequestInterceptor::intercept(
    const ucf::infrastructure::webview::WebRequest& request)
{
    const std::string& url = request.url;

    if (m_resolver && m_resolver->canHandle(url))
    {
        const ResolvedResource resolved = m_resolver->resolve(url);
        if (!resolved.handled)
        {
            return {};
        }

        if (resolved.statusCode == 403)
        {
            return makeFail(403, resolved.errorMessage.empty() ? "forbidden" : resolved.errorMessage);
        }

        return makeRespond(resolved);
    }

    return {};
}

} // namespace ucf::agents
