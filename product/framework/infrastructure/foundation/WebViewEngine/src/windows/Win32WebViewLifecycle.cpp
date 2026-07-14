#include "Win32WebViewImpl.h"

#if defined(_WIN32)

namespace ucf::infrastructure::webview {

using detail::takeCoTaskString;
using detail::utf8ToWide;
using Microsoft::WRL::Make;

namespace {

const wchar_t* const kHostWindowClass = L"UcfWebViewEngineHostWindow";

} // namespace

// -----------------------------------------------------------------------------
// Host window
// -----------------------------------------------------------------------------

LRESULT CALLBACK Win32WebView::Impl::hostWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
    }

    auto* self = reinterpret_cast<Impl*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (self && msg == WM_SIZE)
    {
        self->resizeControllerToClient();
    }
    else if (msg == kUcfResourceResolvedMessage)
    {
        // An off-thread resource resolution came back. Deliver on this (UI)
        // thread and complete the deferral, then free the context.
        auto* pending = reinterpret_cast<PendingResourceResponse*>(lParam);
        if (pending)
        {
            if (pending->alive && pending->alive->load() && pending->impl)
            {
                pending->impl->respondToInterception(pending->args.Get(), pending->result);
                if (pending->deferral)
                {
                    pending->deferral->Complete();
                }
            }
            delete pending;
        }
        return 0;
    }
    return ::DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool Win32WebView::Impl::createHostWindow()
{
    HINSTANCE instance = ::GetModuleHandleW(nullptr);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = &Impl::hostWndProc;
    wc.hInstance = instance;
    wc.lpszClassName = kHostWindowClass;
    // IDC_ARROW is a MAKEINTRESOURCE atom; reinterpret to the wide form since
    // the project is not compiled with UNICODE defined globally.
    wc.hCursor = ::LoadCursorW(nullptr, reinterpret_cast<LPCWSTR>(IDC_ARROW));
    // System-color brush (COLOR_WINDOW + 1) paints the window before the
    // controller draws, avoiding a flash while WebView2 spins up.
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    // RegisterClassEx fails harmlessly if the class already exists (multiple
    // web views in one process); treat ERROR_CLASS_ALREADY_EXISTS as success.
    if (::RegisterClassExW(&wc) == 0)
    {
        const DWORD err = ::GetLastError();
        if (err != ERROR_CLASS_ALREADY_EXISTS)
        {
            return false;
        }
    }

    // Child of the desktop, hidden until the UI reparents it via
    // QWindow::fromWinId(); Qt then drives sizing (delivered here as WM_SIZE).
    hostWindow = ::CreateWindowExW(
        0,
        kHostWindowClass,
        L"",
        WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0, 0, 1, 1,
        ::GetDesktopWindow(),
        nullptr,
        instance,
        this);
    return hostWindow != nullptr;
}

void Win32WebView::Impl::destroyHostWindow()
{
    if (hostWindow)
    {
        ::SetWindowLongPtrW(hostWindow, GWLP_USERDATA, 0);
        ::DestroyWindow(hostWindow);
        hostWindow = nullptr;
    }
}

void Win32WebView::Impl::resizeControllerToClient()
{
    if (!controller || !hostWindow)
    {
        return;
    }
    RECT bounds = {};
    ::GetClientRect(hostWindow, &bounds);
    controller->put_Bounds(bounds);
}

// -----------------------------------------------------------------------------
// Async creation
// -----------------------------------------------------------------------------

bool Win32WebView::Impl::startEnvironmentCreation()
{
    ComPtr<CoreWebView2EnvironmentOptions> envOptions = Make<CoreWebView2EnvironmentOptions>();
    if (!envOptions)
    {
        return false;
    }

    // Register the custom schemes so app:// requests reach the
    // WebResourceRequested handler (the WebView2 analogue of WKURLSchemeHandler).
    std::vector<ComPtr<ICoreWebView2CustomSchemeRegistration>> registrations;
    for (const std::string& scheme : customSchemes)
    {
        auto reg = Make<CoreWebView2CustomSchemeRegistration>(utf8ToWide(scheme).c_str());
        if (!reg)
        {
            continue;
        }
        reg->put_TreatAsSecure(TRUE);
        reg->put_HasAuthorityComponent(TRUE);
        registrations.push_back(reg);
    }
    if (!registrations.empty())
    {
        ComPtr<ICoreWebView2EnvironmentOptions4> options4;
        if (SUCCEEDED(envOptions.As(&options4)) && options4)
        {
            std::vector<ICoreWebView2CustomSchemeRegistration*> raw;
            raw.reserve(registrations.size());
            for (auto& r : registrations)
            {
                raw.push_back(r.Get());
            }
            options4->SetCustomSchemeRegistrations(static_cast<UINT32>(raw.size()), raw.data());
        }
    }

    const std::wstring userDataFolder = utf8ToWide(options.userDataFolder);
    Impl* self = this;
    auto flag = alive;

    const HRESULT hr = ::CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        userDataFolder.empty() ? nullptr : userDataFolder.c_str(),
        envOptions.Get(),
        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [self, flag](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                if (!flag->load())
                {
                    return S_OK;
                }
                if (FAILED(result) || !env)
                {
                    self->owner->emitLoadFailed(static_cast<int>(result), "WebView2 environment creation failed");
                    return S_OK;
                }
                self->onEnvironmentCreated(env);
                return S_OK;
            })
            .Get());

    return SUCCEEDED(hr);
}

void Win32WebView::Impl::onEnvironmentCreated(ICoreWebView2Environment* env)
{
    environment = env;
    Impl* self = this;
    auto flag = alive;

    const HRESULT hr = environment->CreateCoreWebView2Controller(
        hostWindow,
        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
            [self, flag](HRESULT result, ICoreWebView2Controller* ctrl) -> HRESULT {
                if (!flag->load())
                {
                    return S_OK;
                }
                if (FAILED(result) || !ctrl)
                {
                    self->owner->emitLoadFailed(static_cast<int>(result), "WebView2 controller creation failed");
                    return S_OK;
                }
                self->onControllerCreated(ctrl);
                return S_OK;
            })
            .Get());

    if (FAILED(hr))
    {
        owner->emitLoadFailed(static_cast<int>(hr), "WebView2 controller creation could not start");
    }
}

void Win32WebView::Impl::onControllerCreated(ICoreWebView2Controller* ctrl)
{
    controller = ctrl;
    controller->get_CoreWebView2(&webView);
    if (!webView)
    {
        owner->emitLoadFailed(-1, "WebView2 core object unavailable");
        return;
    }

    applySettings();
    wireEvents();
    injectDocumentStartScripts();

    controller->put_IsVisible(TRUE);
    resizeControllerToClient();

    becomeReady();
}

void Win32WebView::Impl::applySettings()
{
    ComPtr<ICoreWebView2Settings> settings;
    if (FAILED(webView->get_Settings(&settings)) || !settings)
    {
        return;
    }
    settings->put_IsWebMessageEnabled(TRUE);
    settings->put_AreDefaultContextMenusEnabled(TRUE);
    settings->put_IsStatusBarEnabled(FALSE);

    if (options.userAgentOverride.has_value() && !options.userAgentOverride->empty())
    {
        ComPtr<ICoreWebView2Settings2> settings2;
        if (SUCCEEDED(settings.As(&settings2)) && settings2)
        {
            settings2->put_UserAgent(utf8ToWide(*options.userAgentOverride).c_str());
        }
    }
}

void Win32WebView::Impl::injectDocumentStartScripts()
{
    for (const std::string& script : options.documentStartScripts)
    {
        if (script.empty())
        {
            continue;
        }
        webView->AddScriptToExecuteOnDocumentCreated(utf8ToWide(script).c_str(), nullptr);
    }
}

void Win32WebView::Impl::wireEvents()
{
    Impl* self = this;
    auto flag = alive;
    EventRegistrationToken token = {};

    webView->add_NavigationStarting(
        Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>(
            [self, flag](ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
                (void)sender;
                if (!flag->load())
                {
                    return S_OK;
                }
                LPWSTR uri = nullptr;
                if (args && SUCCEEDED(args->get_Uri(&uri)) && uri)
                {
                    self->owner->emitNavigationStarted(takeCoTaskString(uri));
                }
                return S_OK;
            })
            .Get(),
        &token);

    webView->add_SourceChanged(
        Microsoft::WRL::Callback<ICoreWebView2SourceChangedEventHandler>(
            [self, flag](ICoreWebView2* sender, ICoreWebView2SourceChangedEventArgs* args) -> HRESULT {
                (void)args;
                if (!flag->load() || !sender)
                {
                    return S_OK;
                }
                LPWSTR uri = nullptr;
                if (SUCCEEDED(sender->get_Source(&uri)) && uri)
                {
                    self->owner->emitUrlChanged(takeCoTaskString(uri));
                }
                return S_OK;
            })
            .Get(),
        &token);

    webView->add_DocumentTitleChanged(
        Microsoft::WRL::Callback<ICoreWebView2DocumentTitleChangedEventHandler>(
            [self, flag](ICoreWebView2* sender, IUnknown* args) -> HRESULT {
                (void)args;
                if (!flag->load() || !sender)
                {
                    return S_OK;
                }
                LPWSTR title = nullptr;
                if (SUCCEEDED(sender->get_DocumentTitle(&title)) && title)
                {
                    self->owner->emitTitleChanged(takeCoTaskString(title));
                }
                return S_OK;
            })
            .Get(),
        &token);

    webView->add_NavigationCompleted(
        Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
            [self, flag](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                (void)sender;
                if (!flag->load() || !args)
                {
                    return S_OK;
                }
                BOOL success = FALSE;
                args->get_IsSuccess(&success);
                if (success)
                {
                    self->owner->emitLoadFinished(true);
                }
                else
                {
                    COREWEBVIEW2_WEB_ERROR_STATUS status = COREWEBVIEW2_WEB_ERROR_STATUS_UNKNOWN;
                    args->get_WebErrorStatus(&status);
                    self->owner->emitLoadFailed(static_cast<int>(status), "navigation failed");
                    self->owner->emitLoadFinished(false);
                }
                return S_OK;
            })
            .Get(),
        &token);

    webView->add_WebMessageReceived(
        Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
            [self, flag](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                (void)sender;
                if (!flag->load() || !args)
                {
                    return S_OK;
                }
                LPWSTR message = nullptr;
                // The page posts strings via window.chrome.webview.postMessage;
                // TryGetWebMessageAsString returns them (JSON payloads included).
                if (SUCCEEDED(args->TryGetWebMessageAsString(&message)) && message)
                {
                    self->owner->emitScriptMessage(self->reportChannel, takeCoTaskString(message));
                }
                return S_OK;
            })
            .Get(),
        &token);

    // Filter every request. Prefer the source-kinds overload (ICoreWebView2_22+)
    // so iframe requests are also intercepted; fall back on older runtimes.
    ComPtr<ICoreWebView2_22> webView22;
    if (SUCCEEDED(webView.As(&webView22)) && webView22)
    {
        webView22->AddWebResourceRequestedFilterWithRequestSourceKinds(
            L"*",
            COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL,
            COREWEBVIEW2_WEB_RESOURCE_REQUEST_SOURCE_KINDS_ALL);
    }
    else
    {
        webView->AddWebResourceRequestedFilter(L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
    }
    webView->add_WebResourceRequested(
        Microsoft::WRL::Callback<ICoreWebView2WebResourceRequestedEventHandler>(
            [self, flag](ICoreWebView2* sender, ICoreWebView2WebResourceRequestedEventArgs* args) -> HRESULT {
                (void)sender;
                if (!flag->load() || !args)
                {
                    return S_OK;
                }
                self->handleWebResourceRequested(args);
                return S_OK;
            })
            .Get(),
        &token);
}

void Win32WebView::Impl::becomeReady()
{
    if (ready.exchange(true))
    {
        return;
    }
    owner->emitWebViewReady();
}

} // namespace ucf::infrastructure::webview

#endif // _WIN32
