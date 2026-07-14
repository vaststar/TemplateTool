#include "GtkWebKitWebViewImpl.h"

#include "InterceptorDispatcher.h"
#include "WebViewSchemeUtils.h"

#if defined(__linux__)

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace ucf::infrastructure::webview {

namespace {

// Convert a WebKitWebView's live URI to std::string ("" when null).
std::string currentUri(WebKitWebView* view)
{
    const gchar* uri = webkit_web_view_get_uri(view);
    return uri ? std::string(uri) : std::string();
}

// --- Navigation signal trampolines ------------------------------------------

void onLoadChanged(WebKitWebView* view, WebKitLoadEvent event, gpointer user_data)
{
    auto* self = static_cast<GtkWebKitWebView*>(user_data);
    switch (event)
    {
        case WEBKIT_LOAD_STARTED:
            self->emitNavigationStarted(currentUri(view));
            break;
        case WEBKIT_LOAD_COMMITTED:
            self->emitUrlChanged(currentUri(view));
            break;
        case WEBKIT_LOAD_FINISHED:
            self->emitLoadFinished(true);
            break;
        default:
            break;
    }
}

gboolean onLoadFailed(WebKitWebView* /*view*/, WebKitLoadEvent /*event*/,
                      const gchar* /*failing_uri*/, GError* error, gpointer user_data)
{
    auto* self = static_cast<GtkWebKitWebView*>(user_data);
    const int code = error ? static_cast<int>(error->code) : -1;
    const std::string message = (error && error->message) ? std::string(error->message) : std::string();
    self->emitLoadFailed(code, message);
    self->emitLoadFinished(false);
    return FALSE; // let WebKit show its default error page
}

void onNotifyTitle(GObject* object, GParamSpec* /*pspec*/, gpointer user_data)
{
    auto* self = static_cast<GtkWebKitWebView*>(user_data);
    auto* view = WEBKIT_WEB_VIEW(object);
    const gchar* title = webkit_web_view_get_title(view);
    self->emitTitleChanged(title ? std::string(title) : std::string());
}

void onNotifyUri(GObject* object, GParamSpec* /*pspec*/, gpointer user_data)
{
    auto* self = static_cast<GtkWebKitWebView*>(user_data);
    auto* view = WEBKIT_WEB_VIEW(object);
    self->emitUrlChanged(currentUri(view));
}

// --- JS->native bridge ------------------------------------------------------
// Each registered channel connects onChannelScriptMessage via a per-channel
// closure so it reports its own channel name.
struct ChannelClosure
{
    GtkWebKitWebView* self;
    std::string channel;
};

void onChannelScriptMessage(WebKitUserContentManager* /*ucm*/, WebKitJavascriptResult* result, gpointer user_data)
{
    auto* closure = static_cast<ChannelClosure*>(user_data);
    if (!closure || !closure->self)
    {
        return;
    }

    JSCValue* value = webkit_javascript_result_get_js_value(result);
    if (!value)
    {
        return;
    }

    std::string payload;
    if (jsc_value_is_string(value))
    {
        gchar* str = jsc_value_to_string(value);
        if (str)
        {
            payload.assign(str);
            g_free(str);
        }
    }
    else
    {
        gchar* json = jsc_value_to_json(value, 0);
        if (json)
        {
            payload.assign(json);
            g_free(json);
        }
    }

    closure->self->emitScriptMessage(closure->channel, payload);
}

void destroyChannelClosure(gpointer data, GClosure* /*closure*/)
{
    delete static_cast<ChannelClosure*>(data);
}

// --- evaluateJavaScript async result ----------------------------------------

struct EvalClosure
{
    JavaScriptResultCallback callback;
};

void onJavaScriptFinished(GObject* source, GAsyncResult* asyncResult, gpointer user_data)
{
    auto* closure = static_cast<EvalClosure*>(user_data);
    std::unique_ptr<EvalClosure> guard(closure);

    auto* view = WEBKIT_WEB_VIEW(source);
    GError* error = nullptr;
    JSCValue* value = webkit_web_view_evaluate_javascript_finish(view, asyncResult, &error);

    if (error)
    {
        if (closure->callback)
        {
            closure->callback(false, "", WebError{ static_cast<int>(error->code),
                                                   error->message ? std::string(error->message) : std::string() });
        }
        g_error_free(error);
        return;
    }

    std::string json = "null";
    if (value)
    {
        gchar* encoded = jsc_value_to_json(value, 0);
        if (encoded)
        {
            json.assign(encoded);
            g_free(encoded);
        }
        g_object_unref(value);
    }

    if (closure->callback)
    {
        closure->callback(true, json, WebError{});
    }
}

} // namespace

// ---------------------------------------------------------------------------
// GtkWebKitWebView
// ---------------------------------------------------------------------------

GtkWebKitWebView::GtkWebKitWebView()
    : m_impl(std::make_unique<Impl>())
{
    m_impl->owner = this;
}

GtkWebKitWebView::~GtkWebKitWebView()
{
    if (m_impl->webView)
    {
        if (m_impl->sigLoadChanged) { g_signal_handler_disconnect(m_impl->webView, m_impl->sigLoadChanged); }
        if (m_impl->sigLoadFailed)  { g_signal_handler_disconnect(m_impl->webView, m_impl->sigLoadFailed); }
        if (m_impl->sigNotifyTitle) { g_signal_handler_disconnect(m_impl->webView, m_impl->sigNotifyTitle); }
        if (m_impl->sigNotifyUri)   { g_signal_handler_disconnect(m_impl->webView, m_impl->sigNotifyUri); }
    }

    // Destroying the top-level window destroys the child WebKitWebView it holds.
    if (m_impl->hostWindow)
    {
        gtk_widget_destroy(m_impl->hostWindow);
        m_impl->hostWindow = nullptr;
        m_impl->webView = nullptr;
    }

    if (m_impl->filterStore)
    {
        g_object_unref(m_impl->filterStore);
        m_impl->filterStore = nullptr;
    }
    if (m_impl->contentManager)
    {
        g_object_unref(m_impl->contentManager);
        m_impl->contentManager = nullptr;
    }
    if (m_impl->context)
    {
        g_object_unref(m_impl->context);
        m_impl->context = nullptr;
    }
    // Note: m_impl->dataManager is owned by the web context; it is released when
    // the context is unref'd above, so it must not be unref'd separately here.
}

bool GtkWebKitWebView::initialize(const WebViewInitOptions& options)
{
    if (m_impl->initialized)
    {
        return false;
    }

    // GTK must be initialized before any widget is created. Safe to call more
    // than once; a no-op after the first successful init.
    if (!gtk_init_check(nullptr, nullptr))
    {
        return false;
    }

    // Validate, deduplicate, and default the custom schemes (shared helper).
    std::optional<std::vector<std::string>> schemes =
        scheme_utils::normalizeCustomSchemes(options.customSchemes);
    if (!schemes)
    {
        return false;
    }

    m_impl->initialized = true;
    m_impl->customSchemes = *schemes;

    // --- Web context (optionally persistent per-instance data store) --------
    if (!options.userDataFolder.empty())
    {
        std::string baseCache = options.userDataFolder + "/cache";
        std::string baseData = options.userDataFolder + "/data";
        m_impl->dataManager = webkit_website_data_manager_new(
            "base-cache-directory", baseCache.c_str(),
            "base-data-directory", baseData.c_str(),
            nullptr);
        m_impl->context = webkit_web_context_new_with_website_data_manager(m_impl->dataManager);
    }
    else
    {
        m_impl->context = webkit_web_context_new();
    }

    // --- Custom scheme handlers (must be registered on the context) ---------
    for (const std::string& scheme : m_impl->customSchemes)
    {
        webkit_web_context_register_uri_scheme(
            m_impl->context, scheme.c_str(), detail::onUriSchemeRequest, this, nullptr);
    }

    // --- User content manager: document-start scripts + message channels ----
    m_impl->contentManager = webkit_user_content_manager_new();

    for (const std::string& script : options.documentStartScripts)
    {
        WebKitUserScript* userScript = webkit_user_script_new(
            script.c_str(),
            WEBKIT_USER_CONTENT_INJECT_TOP_FRAME,
            WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
            nullptr, nullptr);
        webkit_user_content_manager_add_script(m_impl->contentManager, userScript);
        webkit_user_script_unref(userScript);
    }

    for (const std::string& channel : options.scriptChannels)
    {
        if (channel.empty())
        {
            continue;
        }
        // JS side posts via window.webkit.messageHandlers.<channel>.postMessage(...)
        webkit_user_content_manager_register_script_message_handler(
            m_impl->contentManager, channel.c_str());

        std::string detailed = std::string("script-message-received::") + channel;
        auto* closure = new ChannelClosure{ this, channel };
        g_signal_connect_data(m_impl->contentManager, detailed.c_str(),
                              G_CALLBACK(onChannelScriptMessage), closure,
                              destroyChannelClosure, static_cast<GConnectFlags>(0));
    }

    // --- Web view -----------------------------------------------------------
    // Build via properties so the view binds to BOTH our custom context (custom
    // scheme handlers live there) and our user content manager in one shot.
    m_impl->webView = WEBKIT_WEB_VIEW(g_object_new(
        WEBKIT_TYPE_WEB_VIEW,
        "web-context", m_impl->context,
        "user-content-manager", m_impl->contentManager,
        nullptr));

    // User-Agent override.
    if (options.userAgentOverride.has_value() && !options.userAgentOverride->empty())
    {
        WebKitSettings* settings = webkit_web_view_get_settings(m_impl->webView);
        webkit_settings_set_user_agent(settings, options.userAgentOverride->c_str());
    }

    if (options.allowPopups)
    {
        WebKitSettings* settings = webkit_web_view_get_settings(m_impl->webView);
        webkit_settings_set_javascript_can_open_windows_automatically(settings, TRUE);
    }

    // --- Signals ------------------------------------------------------------
    m_impl->sigLoadChanged = g_signal_connect(m_impl->webView, "load-changed",
                                              G_CALLBACK(onLoadChanged), this);
    m_impl->sigLoadFailed = g_signal_connect(m_impl->webView, "load-failed",
                                             G_CALLBACK(onLoadFailed), this);
    m_impl->sigNotifyTitle = g_signal_connect(m_impl->webView, "notify::title",
                                              G_CALLBACK(onNotifyTitle), this);
    m_impl->sigNotifyUri = g_signal_connect(m_impl->webView, "notify::uri",
                                            G_CALLBACK(onNotifyUri), this);

    // --- Host window (embedded into Qt via its X11 XID) ---------------------
    m_impl->hostWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated(GTK_WINDOW(m_impl->hostWindow), FALSE);
    gtk_container_add(GTK_CONTAINER(m_impl->hostWindow), GTK_WIDGET(m_impl->webView));

    // Realize so the GdkWindow (and its X11 backing window) exists; do NOT
    // gtk_widget_show the top-level as a standalone window -- Qt maps it after
    // reparenting via createWindowContainer.
    gtk_widget_show(GTK_WIDGET(m_impl->webView));
    gtk_widget_realize(m_impl->hostWindow);

    GdkWindow* gdkWindow = gtk_widget_get_window(m_impl->hostWindow);
    if (gdkWindow && GDK_IS_X11_WINDOW(gdkWindow))
    {
        m_impl->xid = gdk_x11_window_get_xid(gdkWindow);
    }

    // --- Network access policy (fail-closed when present) -------------------
    // When a policy imposes an actual restriction, applyNetworkPolicy() compiles
    // and installs the content filter asynchronously and flips ready afterwards
    // (fail-closed). Otherwise we become ready immediately.
    const bool deferredReady = options.networkPolicy.has_value()
        && detail::applyNetworkPolicy(this, m_impl.get(), *options.networkPolicy, options.userDataFolder);

    if (!deferredReady)
    {
        m_impl->ready = true;
        // Deliver readiness on the next main-loop turn so late-registered
        // callbacks still receive it (matches the async backends).
        g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,
            [](gpointer data) -> gboolean {
                static_cast<GtkWebKitWebView*>(data)->emitWebViewReady();
                return G_SOURCE_REMOVE;
            },
            this, nullptr);
    }

    return true;
}

bool GtkWebKitWebView::isReady() const
{
    return m_impl->ready;
}

void GtkWebKitWebView::loadUrl(const std::string& url)
{
    if (!m_impl->webView || url.empty())
    {
        return;
    }
    webkit_web_view_load_uri(m_impl->webView, url.c_str());
}

void GtkWebKitWebView::reload()
{
    if (m_impl->webView)
    {
        webkit_web_view_reload(m_impl->webView);
    }
}

void GtkWebKitWebView::stop()
{
    if (m_impl->webView)
    {
        webkit_web_view_stop_loading(m_impl->webView);
    }
}

void GtkWebKitWebView::evaluateJavaScript(const std::string& js, JavaScriptResultCallback callback)
{
    if (!m_impl->webView || js.empty())
    {
        if (callback)
        {
            callback(false, "", WebError{ -1, "web view not ready or empty script" });
        }
        return;
    }

    auto* closure = new EvalClosure{ std::move(callback) };
    webkit_web_view_evaluate_javascript(
        m_impl->webView, js.c_str(), static_cast<gssize>(js.size()),
        nullptr, nullptr, nullptr, onJavaScriptFinished, closure);
}

InterceptorId GtkWebKitWebView::addRequestInterceptor(std::shared_ptr<IRequestInterceptor> interceptor)
{
    return m_impl->dispatcher.add(std::move(interceptor));
}

void GtkWebKitWebView::removeRequestInterceptor(InterceptorId id)
{
    m_impl->dispatcher.remove(id);
}

void GtkWebKitWebView::clearRequestInterceptors()
{
    m_impl->dispatcher.clear();
}

InterceptorDispatcher& GtkWebKitWebView::internalDispatcher()
{
    return m_impl->dispatcher;
}

NativeHostHandle GtkWebKitWebView::nativeHostHandle() const
{
    return static_cast<NativeHostHandle>(m_impl->xid);
}

} // namespace ucf::infrastructure::webview

#else

// Fallback stub for non-Linux builds (this file is only compiled on Linux via
// CMake, but the guard keeps it self-contained).
namespace ucf::infrastructure::webview {

struct GtkWebKitWebView::Impl {};

GtkWebKitWebView::GtkWebKitWebView() : m_impl(std::make_unique<Impl>()) {}
GtkWebKitWebView::~GtkWebKitWebView() = default;
bool GtkWebKitWebView::initialize(const WebViewInitOptions&) { return false; }
bool GtkWebKitWebView::isReady() const { return false; }
void GtkWebKitWebView::loadUrl(const std::string&) {}
void GtkWebKitWebView::reload() {}
void GtkWebKitWebView::stop() {}
void GtkWebKitWebView::evaluateJavaScript(const std::string&, JavaScriptResultCallback) {}
InterceptorId GtkWebKitWebView::addRequestInterceptor(std::shared_ptr<IRequestInterceptor>) { return 0; }
void GtkWebKitWebView::removeRequestInterceptor(InterceptorId) {}
void GtkWebKitWebView::clearRequestInterceptors() {}
InterceptorDispatcher& GtkWebKitWebView::internalDispatcher()
{
    static InterceptorDispatcher dummy;
    return dummy;
}
NativeHostHandle GtkWebKitWebView::nativeHostHandle() const { return 0; }

} // namespace ucf::infrastructure::webview

#endif // __linux__
