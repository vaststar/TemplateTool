#include "GtkWebKitWebViewImpl.h"

#include "InterceptorDispatcher.h"

#if defined(__linux__)

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace ucf::infrastructure::webview {

namespace {

// ---------------------------------------------------------------------------
// Network access policy -> WebKitUserContentFilter JSON.
//
// WebKitUserContentFilter consumes the exact same rule schema as the macOS
// WKContentRuleList, so this mirrors buildContentRuleListJson() in the macOS
// backend. Keeping the two in sync means a single NetworkAccessPolicy produces
// equivalent enforcement on both platforms.
// ---------------------------------------------------------------------------

std::string escapeHostForRegex(const std::string& host)
{
    std::string out;
    out.reserve(host.size() * 2);
    for (char c : host)
    {
        switch (c)
        {
            case '.': case '\\': case '+': case '*': case '?':
            case '(': case ')': case '[': case ']': case '{': case '}':
            case '^': case '$': case '|': case '/':
                out.push_back('\\');
                break;
            default:
                break;
        }
        out.push_back(c);
    }
    return out;
}

std::string jsonEscape(const std::string& value)
{
    std::string out;
    out.reserve(value.size() + 8);
    for (char c : value)
    {
        if (c == '\\' || c == '"')
        {
            out.push_back('\\');
        }
        out.push_back(c);
    }
    return out;
}

std::string hostTailRegex(const std::string& host, bool includeSubdomains)
{
    std::string tail;
    if (includeSubdomains)
    {
        tail += "([a-z0-9-]+\\.)*";
    }
    tail += escapeHostForRegex(host);
    tail += "(:[0-9]+)?(/|$)";
    return tail;
}

std::string buildContentRuleListJson(const NetworkAccessPolicy& policy)
{
    using DefaultAction = NetworkAccessPolicy::DefaultAction;

    auto ruleForFilter = [](const std::string& urlFilter, const char* actionType) {
        std::string rule = "{\"trigger\":{\"url-filter\":\"";
        rule += jsonEscape(urlFilter);
        rule += "\",\"url-filter-is-case-sensitive\":false},\"action\":{\"type\":\"";
        rule += actionType;
        rule += "\"}}";
        return rule;
    };

    std::vector<std::string> rules;
    const bool blockAll = (policy.defaultAction == DefaultAction::Block);

    if (blockAll)
    {
        rules.push_back(ruleForFilter("^https?://", "block"));
        rules.push_back(ruleForFilter("^wss?://", "block"));

        for (const std::string& host : policy.allowedHosts)
        {
            if (host.empty()) { continue; }
            const std::string tail = hostTailRegex(host, policy.includeSubdomains);
            rules.push_back(ruleForFilter("^https?://" + tail, "ignore-previous-rules"));
            rules.push_back(ruleForFilter("^wss?://" + tail, "ignore-previous-rules"));
        }
    }

    for (const std::string& host : policy.blockedHosts)
    {
        if (host.empty()) { continue; }
        const std::string tail = hostTailRegex(host, policy.includeSubdomains);
        rules.push_back(ruleForFilter("^https?://" + tail, "block"));
        rules.push_back(ruleForFilter("^wss?://" + tail, "block"));
    }

    if (rules.empty())
    {
        return {};
    }

    std::string json = "[";
    for (size_t i = 0; i < rules.size(); ++i)
    {
        if (i != 0) { json += ","; }
        json += rules[i];
    }
    json += "]";
    return json;
}

// FNV-1a hash of the rule JSON, used as a stable content-filter identifier so
// identical policies compile once and are reused across runs.
std::string stableRuleIdentifier(const std::string& json)
{
    std::uint64_t h = 1469598103934665603ULL;
    for (unsigned char c : json)
    {
        h ^= c;
        h *= 1099511628211ULL;
    }
    std::ostringstream oss;
    oss << "miniapp_netpolicy_" << std::hex << h;
    return oss.str();
}

// --- Network policy filter installation -------------------------------------

struct FilterSaveClosure
{
    GtkWebKitWebView* self;
    GtkWebKitWebView::Impl* impl;
};

void onContentFilterSaved(GObject* source, GAsyncResult* asyncResult, gpointer user_data)
{
    auto* closure = static_cast<FilterSaveClosure*>(user_data);
    std::unique_ptr<FilterSaveClosure> guard(closure);

    auto* store = WEBKIT_USER_CONTENT_FILTER_STORE(source);
    GError* error = nullptr;
    WebKitUserContentFilter* filter =
        webkit_user_content_filter_store_save_finish(store, asyncResult, &error);

    if (filter && closure->impl && closure->impl->contentManager)
    {
        webkit_user_content_manager_add_filter(closure->impl->contentManager, filter);
        webkit_user_content_filter_unref(filter);
        closure->impl->ready = true;
        closure->self->emitWebViewReady();
    }
    else
    {
        // Fail-closed: stay not-ready and report the compile failure.
        const std::string msg = error && error->message ? std::string(error->message)
                                                         : std::string("content filter compilation failed");
        closure->self->emitLoadFailed(kWebErrorContentPolicyCompileFailed, msg);
    }

    if (error)
    {
        g_error_free(error);
    }
}

} // namespace

namespace detail {

void onUriSchemeRequest(WebKitURISchemeRequest* request, gpointer user_data)
{
    auto* self = static_cast<GtkWebKitWebView*>(user_data);

    WebRequest webRequest;
    const gchar* uri = webkit_uri_scheme_request_get_uri(request);
    webRequest.url = uri ? std::string(uri) : std::string();

    const gchar* method = webkit_uri_scheme_request_get_http_method(request);
    webRequest.method = method ? std::string(method) : std::string("GET");

    if (SoupMessageHeaders* headers = webkit_uri_scheme_request_get_http_headers(request))
    {
        SoupMessageHeadersIter iter;
        soup_message_headers_iter_init(&iter, headers);
        const char* name = nullptr;
        const char* value = nullptr;
        while (soup_message_headers_iter_next(&iter, &name, &value))
        {
            if (name)
            {
                webRequest.headers[std::string(name)] = value ? std::string(value) : std::string();
            }
        }
    }

    // Resolve synchronously on the main thread. Interceptors that read local
    // files do a blocking read here; acceptable for the local app:// resources
    // this scheme serves. (The macOS backend resolves off-thread; that can be
    // added later if large payloads become an issue.)
    const InterceptResult result = self->internalDispatcher().dispatch(webRequest);

    switch (result.action)
    {
        case InterceptAction::Respond:
        {
            if (!result.response)
            {
                GError* error = g_error_new_literal(g_quark_from_string("WebViewEngine"), 500,
                                                    "Respond action with no response");
                webkit_uri_scheme_request_finish_error(request, error);
                g_error_free(error);
                break;
            }

            const WebResponse& response = *result.response;

            // Copy the body into a GBytes-backed input stream; WebKit takes a
            // reference to the stream and drains it asynchronously.
            gpointer bodyCopy = nullptr;
            gsize bodyLen = response.body.size();
            if (bodyLen > 0)
            {
                bodyCopy = g_memdup2(response.body.data(), bodyLen);
            }
            GInputStream* stream = g_memory_input_stream_new_from_data(
                bodyCopy, static_cast<gssize>(bodyLen), bodyCopy ? g_free : nullptr);

            const char* mime = response.mimeType.empty() ? "application/octet-stream"
                                                          : response.mimeType.c_str();
            webkit_uri_scheme_request_finish(request, stream, static_cast<gint64>(bodyLen), mime);
            g_object_unref(stream);
            break;
        }
        case InterceptAction::Fail:
        {
            const int code = result.error ? result.error->code : 500;
            const std::string msg = result.error ? result.error->message : std::string("Unknown error");
            GError* error = g_error_new_literal(g_quark_from_string("WebViewEngine"), code, msg.c_str());
            webkit_uri_scheme_request_finish_error(request, error);
            g_error_free(error);
            break;
        }
        case InterceptAction::Continue:
        default:
        {
            // No interceptor served this custom-scheme request; the engine owns
            // the full response for custom schemes, so Continue cannot proceed.
            GError* error = g_error_new_literal(g_quark_from_string("WebViewEngine"), 501,
                                                "custom-scheme request was not handled");
            webkit_uri_scheme_request_finish_error(request, error);
            g_error_free(error);
            break;
        }
    }
}

bool applyNetworkPolicy(GtkWebKitWebView* self,
                        GtkWebKitWebView::Impl* impl,
                        const NetworkAccessPolicy& policy,
                        const std::string& userDataFolder)
{
    const std::string ruleJson = buildContentRuleListJson(policy);
    if (ruleJson.empty())
    {
        // Policy imposes no restriction (e.g. default Allow with no blocked
        // hosts); nothing to install, so the caller becomes ready immediately.
        return false;
    }

    // Compile + install the content filter, then flip ready. The store path
    // lives under the user-data folder when available, else a temp dir.
    const std::string storePath = userDataFolder.empty()
        ? std::string(g_get_tmp_dir()) + "/miniapp_content_filters"
        : userDataFolder + "/content_filters";
    impl->filterStore = webkit_user_content_filter_store_new(storePath.c_str());

    const std::string ruleId = stableRuleIdentifier(ruleJson);
    GBytes* source = g_bytes_new(ruleJson.data(), ruleJson.size());
    auto* closure = new FilterSaveClosure{ self, impl };
    webkit_user_content_filter_store_save(
        impl->filterStore, ruleId.c_str(), source, nullptr,
        onContentFilterSaved, closure);
    g_bytes_unref(source);

    return true;
}

} // namespace detail

} // namespace ucf::infrastructure::webview

#endif // __linux__
