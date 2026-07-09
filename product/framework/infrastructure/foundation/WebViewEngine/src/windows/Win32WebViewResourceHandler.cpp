#include "Win32WebViewImpl.h"

#if defined(_WIN32)

namespace ucf::infrastructure::webview {

using detail::takeCoTaskString;
using detail::utf8ToWide;

namespace {

std::string schemeOf(const std::string& url)
{
    const auto pos = url.find("://");
    if (pos == std::string::npos)
    {
        return std::string();
    }
    std::string scheme = url.substr(0, pos);
    for (char& c : scheme)
    {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return scheme;
}

std::string hostOf(const std::string& url)
{
    const auto schemePos = url.find("://");
    if (schemePos == std::string::npos)
    {
        return std::string();
    }
    const size_t start = schemePos + 3;
    size_t end = start;
    while (end < url.size() && url[end] != '/' && url[end] != ':' && url[end] != '?' && url[end] != '#')
    {
        ++end;
    }
    std::string host = url.substr(start, end - start);
    for (char& c : host)
    {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return host;
}

bool isRemoteScheme(const std::string& scheme)
{
    return scheme == "http" || scheme == "https" || scheme == "ws" || scheme == "wss";
}

bool hostMatches(const std::string& host, const std::string& entry, bool includeSubdomains)
{
    if (entry.empty())
    {
        return false;
    }
    if (host == entry)
    {
        return true;
    }
    if (includeSubdomains && host.size() > entry.size() + 1)
    {
        const std::string suffix = "." + entry;
        return host.compare(host.size() - suffix.size(), suffix.size(), suffix) == 0;
    }
    return false;
}

// Mirrors the WKContentRuleList ordering: blocked hosts take precedence, then
// allowed hosts, then the default action for everything else.
bool isRemoteAllowed(const std::string& host, const NetworkAccessPolicy& policy)
{
    for (const std::string& blocked : policy.blockedHosts)
    {
        if (hostMatches(host, blocked, policy.includeSubdomains))
        {
            return false;
        }
    }
    for (const std::string& allowed : policy.allowedHosts)
    {
        if (hostMatches(host, allowed, policy.includeSubdomains))
        {
            return true;
        }
    }
    return policy.defaultAction == NetworkAccessPolicy::DefaultAction::Allow;
}

std::vector<std::uint8_t> readAllFromStream(IStream* stream)
{
    std::vector<std::uint8_t> data;
    if (!stream)
    {
        return data;
    }
    constexpr ULONG kChunk = 8192;
    std::uint8_t buffer[kChunk];
    ULONG read = 0;
    while (SUCCEEDED(stream->Read(buffer, kChunk, &read)) && read > 0)
    {
        data.insert(data.end(), buffer, buffer + read);
        read = 0;
    }
    return data;
}

} // namespace

// -----------------------------------------------------------------------------
// WebResourceRequested: custom scheme + network policy
// -----------------------------------------------------------------------------

bool Win32WebView::Impl::isCustomScheme(const std::string& scheme) const
{
    for (const std::string& s : customSchemes)
    {
        if (scheme == s)
        {
            return true;
        }
    }
    return false;
}

void Win32WebView::Impl::handleWebResourceRequested(ICoreWebView2WebResourceRequestedEventArgs* args)
{
    ComPtr<ICoreWebView2WebResourceRequest> request;
    if (FAILED(args->get_Request(&request)) || !request)
    {
        return;
    }

    LPWSTR uriRaw = nullptr;
    if (FAILED(request->get_Uri(&uriRaw)) || !uriRaw)
    {
        return;
    }
    const std::string url = takeCoTaskString(uriRaw);
    const std::string scheme = schemeOf(url);

    // Remote requests obey the network policy (fail-closed); registered custom
    // schemes go through the interceptor chain; anything else is left to WebView2.
    if (isRemoteScheme(scheme))
    {
        applyRemotePolicy(args, url);
        return;
    }
    if (!isCustomScheme(scheme))
    {
        return;
    }

    respondToInterception(args, dispatcher.dispatch(buildWebRequest(request.Get(), url)));
}

void Win32WebView::Impl::applyRemotePolicy(
    ICoreWebView2WebResourceRequestedEventArgs* args, const std::string& url)
{
    // No policy, or the host is allowed: let WebView2 perform the fetch.
    if (!options.networkPolicy.has_value() || isRemoteAllowed(hostOf(url), *options.networkPolicy))
    {
        return;
    }
    if (ComPtr<ICoreWebView2WebResourceResponse> blocked = makeResponse(nullptr, 403, L"Blocked", L""))
    {
        args->put_Response(blocked.Get());
    }
}

WebRequest Win32WebView::Impl::buildWebRequest(
    ICoreWebView2WebResourceRequest* request, const std::string& url) const
{
    WebRequest webRequest;
    webRequest.url = url;

    LPWSTR methodRaw = nullptr;
    webRequest.method =
        (SUCCEEDED(request->get_Method(&methodRaw)) && methodRaw) ? takeCoTaskString(methodRaw) : "GET";

    ComPtr<ICoreWebView2HttpRequestHeaders> headers;
    if (SUCCEEDED(request->get_Headers(&headers)) && headers)
    {
        ComPtr<ICoreWebView2HttpHeadersCollectionIterator> it;
        if (SUCCEEDED(headers->GetIterator(&it)) && it)
        {
            BOOL hasCurrent = FALSE;
            while (SUCCEEDED(it->get_HasCurrentHeader(&hasCurrent)) && hasCurrent)
            {
                LPWSTR name = nullptr;
                LPWSTR value = nullptr;
                if (SUCCEEDED(it->GetCurrentHeader(&name, &value)))
                {
                    // takeCoTaskString frees both; a header name is never empty.
                    const std::string key = takeCoTaskString(name);
                    const std::string val = takeCoTaskString(value);
                    if (!key.empty())
                    {
                        webRequest.headers[key] = val;
                    }
                }

                BOOL hasNext = FALSE;
                if (FAILED(it->MoveNext(&hasNext)) || !hasNext)
                {
                    break;
                }
            }
        }
    }

    ComPtr<IStream> content;
    if (SUCCEEDED(request->get_Content(&content)) && content)
    {
        webRequest.body = readAllFromStream(content.Get());
    }
    return webRequest;
}

void Win32WebView::Impl::respondToInterception(
    ICoreWebView2WebResourceRequestedEventArgs* args, const InterceptResult& result)
{
    if (result.action == InterceptAction::Continue)
    {
        // A registered custom scheme that no interceptor served. WebView2 cannot
        // fetch custom schemes itself, so surface a 404.
        if (ComPtr<ICoreWebView2WebResourceResponse> notFound = makeResponse(nullptr, 404, L"Not Found", L""))
        {
            args->put_Response(notFound.Get());
        }
        return;
    }

    if (result.action == InterceptAction::Fail)
    {
        const int code = result.error ? result.error->code : 500;
        if (ComPtr<ICoreWebView2WebResourceResponse> failed = makeResponse(nullptr, code, L"Error", L""))
        {
            args->put_Response(failed.Get());
        }
        return;
    }

    if (!result.response)
    {
        return;
    }
    const WebResponse& response = *result.response;

    ComPtr<IStream> bodyStream;
    if (!response.body.empty())
    {
        bodyStream.Attach(::SHCreateMemStream(response.body.data(), static_cast<UINT>(response.body.size())));
    }

    std::wstring headerText;
    for (const auto& [key, value] : response.headers)
    {
        headerText += utf8ToWide(key) + L": " + utf8ToWide(value) + L"\r\n";
    }
    if (!response.mimeType.empty())
    {
        headerText += L"Content-Type: " + utf8ToWide(response.mimeType) + L"\r\n";
    }

    if (ComPtr<ICoreWebView2WebResourceResponse> webResponse =
            makeResponse(bodyStream.Get(), response.statusCode, L"OK", headerText))
    {
        args->put_Response(webResponse.Get());
    }
}

ComPtr<ICoreWebView2WebResourceResponse> Win32WebView::Impl::makeResponse(
    IStream* body, int statusCode, const wchar_t* reasonPhrase, const std::wstring& headers) const
{
    ComPtr<ICoreWebView2WebResourceResponse> response;
    if (environment)
    {
        environment->CreateWebResourceResponse(body, statusCode, reasonPhrase, headers.c_str(), &response);
    }
    return response;
}

} // namespace ucf::infrastructure::webview

#endif // _WIN32
