#include "GtkWebKitWebViewImpl.h"

#include "InterceptorDispatcher.h"
#include "WebViewContentPolicy.h"

#if defined(__linux__)

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace ucf::infrastructure::webview {

namespace {

// FNV-1a hash of the rule JSON, used as a stable content-filter identifier so
// identical policies compile once and are reused across runs.
std::string stableRuleIdentifier(const std::string& json)
{
    return "miniapp_netpolicy_" + content_policy::ruleListHashHex(json);
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
        closure->self->markReady();
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
    const std::string ruleJson = content_policy::buildContentRuleListJson(policy);
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
