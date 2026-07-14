#pragma once

#include "Win32WebView.h"

#if defined(_WIN32)

// clang-format off
#include <windows.h>
#include <shlwapi.h>
#include <wrl.h>
#include <wrl/event.h>
#include "WebView2.h"
#include "WebView2EnvironmentOptions.h"
// clang-format on

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <ucf/Utilities/StringUtils/UnicodeString.h>

#include "InterceptorDispatcher.h"

namespace ucf::infrastructure::webview {

using Microsoft::WRL::ComPtr;

namespace detail {

// Convert UTF-8 to the platform wide encoding, delegating to the shared
// UnicodeString utility.
inline std::wstring utf8ToWide(const std::string& utf8)
{
    return ucf::utilities::UnicodeString::fromUtf8(utf8).toWide();
}

inline std::string wideToUtf8(const wchar_t* wide)
{
    return ucf::utilities::UnicodeString::fromWide(wide).toStdString();
}

// Convert a WebView2-allocated wide string to UTF-8 and free it. WebView2 hands
// back CoTaskMem-owned strings the caller must release; centralizing the
// "read then free" here keeps individual call sites from leaking.
inline std::string takeCoTaskString(LPWSTR owned)
{
    if (!owned)
    {
        return std::string();
    }
    std::string result = wideToUtf8(owned);
    ::CoTaskMemFree(owned);
    return result;
}

} // namespace detail

// Private implementation shared between the two Win32WebView translation units:
//   * Win32WebView.cpp            - lifecycle, host window, async creation
//   * Win32WebViewInterceptor.cpp - WebResourceRequested / network policy
class Win32WebView::Impl
{
public:
    explicit Impl(Win32WebView* owner)
        : owner(owner)
        , alive(std::make_shared<std::atomic<bool>>(true))
    {
    }

    Win32WebView* owner = nullptr;
    std::shared_ptr<std::atomic<bool>> alive;

    HWND hostWindow = nullptr;
    ComPtr<ICoreWebView2Environment> environment;
    ComPtr<ICoreWebView2Controller> controller;
    ComPtr<ICoreWebView2> webView;

    InterceptorDispatcher dispatcher;
    WebViewInitOptions options;
    std::vector<std::string> customSchemes;
    std::string reportChannel; // channel name reported for WebView2 messages

    bool initialized = false;
    std::atomic<bool> ready { false };

    // --- host window + async creation (Win32WebView.cpp) ---
    bool createHostWindow();
    void destroyHostWindow();
    void resizeControllerToClient();

    bool startEnvironmentCreation();
    void onEnvironmentCreated(ICoreWebView2Environment* env);
    void onControllerCreated(ICoreWebView2Controller* ctrl);
    void applySettings();
    void injectDocumentStartScripts();
    void wireEvents();
    void becomeReady();

    // --- request interception (Win32WebViewInterceptor.cpp) ---
    void handleWebResourceRequested(ICoreWebView2WebResourceRequestedEventArgs* args);
    void applyRemotePolicy(ICoreWebView2WebResourceRequestedEventArgs* args, const std::string& url);
    WebRequest buildWebRequest(ICoreWebView2WebResourceRequest* request, const std::string& url) const;
    void respondToInterception(ICoreWebView2WebResourceRequestedEventArgs* args, const InterceptResult& result);
    ComPtr<ICoreWebView2WebResourceResponse> makeResponse(
        IStream* body, int statusCode, const wchar_t* reasonPhrase, const std::wstring& headers) const;
    bool isCustomScheme(const std::string& scheme) const;

    // Thread-pool entry that resolves a custom-scheme resource off the UI thread,
    // then posts the result back to the host window for delivery.
    static void CALLBACK resolveOnThreadPool(PTP_CALLBACK_INSTANCE instance, void* context);

    static LRESULT CALLBACK hostWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

// Custom window message to marshal an off-thread resource resolution back to the
// UI thread. lParam is an owning PendingResourceResponse* the handler deletes.
inline constexpr UINT kUcfResourceResolvedMessage = WM_APP + 1;

// Context carried from the request handler, through the thread-pool resolve, and
// back to the UI thread. COM members are only touched on the UI thread; the
// thread pool only reads request/interceptors and writes result.
struct PendingResourceResponse
{
    ComPtr<ICoreWebView2WebResourceRequestedEventArgs> args;
    ComPtr<ICoreWebView2Deferral> deferral;
    WebRequest request;
    std::vector<std::shared_ptr<IRequestInterceptor>> interceptors;
    InterceptResult result;
    std::shared_ptr<std::atomic<bool>> alive;
    Win32WebView::Impl* impl = nullptr;
    HWND hostWindow = nullptr;
};

} // namespace ucf::infrastructure::webview

#endif // _WIN32
