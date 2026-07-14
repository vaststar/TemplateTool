#pragma once

#include "GtkWebKitWebView.h"

#if defined(__linux__)

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <jsc/jsc.h>
#include <gdk/gdkx.h>

#include <string>
#include <vector>

#include "InterceptorDispatcher.h"

namespace ucf::infrastructure::webview {

// Private implementation shared between the two GtkWebKitWebView translation
// units:
//   * GtkWebKitWebView.cpp                - lifecycle, host window, navigation
//                                           signals, JS evaluation
//   * GtkWebKitWebViewResourceHandler.cpp - custom-scheme handling + network
//                                           access policy (content filter)
struct GtkWebKitWebView::Impl
{
    GtkWebKitWebView* owner = nullptr;

    GtkWidget* hostWindow = nullptr;   // top-level GtkWindow, embedded via XID
    WebKitWebView* webView = nullptr;  // owned by hostWindow container
    WebKitWebContext* context = nullptr;
    WebKitUserContentManager* contentManager = nullptr;
    WebKitWebsiteDataManager* dataManager = nullptr;
    WebKitUserContentFilterStore* filterStore = nullptr;

    InterceptorDispatcher dispatcher;
    std::vector<std::string> customSchemes;

    bool initialized = false;
    bool ready = false;

    // Connected signal handler ids (for clean disconnect on teardown).
    gulong sigLoadChanged = 0;
    gulong sigLoadFailed = 0;
    gulong sigNotifyTitle = 0;
    gulong sigNotifyUri = 0;

    Window xid = 0;
};

// Cross-translation-unit hooks implemented in GtkWebKitWebViewResourceHandler.cpp
// and used from initialize() in GtkWebKitWebView.cpp.
namespace detail {

// WebKitURIScheme callback: dispatches a custom-scheme request through the
// interceptor chain and finishes it (Respond/Fail/unhandled). Registered on the
// web context for every custom scheme.
void onUriSchemeRequest(WebKitURISchemeRequest* request, gpointer user_data);

// Compile the policy into a WebKitUserContentFilter and install it, deferring
// readiness (fail-closed) until the filter is active. Returns true when the
// caller should wait for the async install (i.e. a restriction was applied), or
// false when the policy imposes no restriction and the caller may become ready
// immediately.
bool applyNetworkPolicy(GtkWebKitWebView* self,
                        GtkWebKitWebView::Impl* impl,
                        const NetworkAccessPolicy& policy,
                        const std::string& userDataFolder);

} // namespace detail

} // namespace ucf::infrastructure::webview

#endif // __linux__
