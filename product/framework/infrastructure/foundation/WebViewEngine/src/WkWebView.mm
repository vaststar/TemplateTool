#include "WkWebView.h"
#include "InterceptorDispatcher.h"

#if defined(Q_OS_MACOS) || defined(__APPLE__)

#import <AppKit/AppKit.h>
#import <WebKit/WebKit.h>

#include <string>
#include <sstream>
#include <algorithm>
#include <cstdint>
#include <vector>

// Objective-C bridge: handles WKWebView callbacks and delegates them to the C++ WkWebView.
// Must be declared at global scope, not inside namespace.
@interface WkWebViewHelper : NSObject <WKNavigationDelegate, WKUIDelegate, WKScriptMessageHandler, WKURLSchemeHandler>
@property (nonatomic, assign) void* backend;  // void* to avoid circular C++/Obj-C dependency
@end

namespace ucf::infrastructure::webview {
class WkWebView;
class IWebViewCallback;
} // forward declarations

namespace {

// A valid custom scheme is a non-empty lowercase string of [a-z0-9+.-]
// starting with [a-z], and it must not collide with a system scheme WK
// refuses to hand off to a custom handler.
bool isValidCustomScheme(const std::string& scheme)
{
    if (scheme.empty()) { return false; }
    auto isSchemeChar = [](char c) {
        return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
               c == '+' || c == '.' || c == '-';
    };
    if (!(scheme.front() >= 'a' && scheme.front() <= 'z')) { return false; }
    for (char c : scheme)
    {
        if (!isSchemeChar(c)) { return false; }
    }
    static const char* kReserved[] = {
        "http", "https", "file", "ftp", "ws", "wss",
        "about", "data", "blob", "javascript"
    };
    for (const char* r : kReserved)
    {
        if (scheme == r) { return false; }
    }
    return true;
}

// Escape a host so it can be embedded literally inside a WKContentRuleList
// url-filter regular expression.
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

// JSON-escape a string (only backslash and double-quote need handling here,
// as regex fragments contain no control characters).
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

// Regex tail matching "<optional subdomains>host<optional port>(path|end)".
std::string hostTailRegex(const std::string& host, bool includeSubdomains)
{
    std::string tail;
    if (includeSubdomains)
    {
        tail += "([a-z0-9-]+\\.)*"; // one backslash in the actual string
    }
    tail += escapeHostForRegex(host);
    tail += "(:[0-9]+)?(/|$)";
    return tail;
}

// Build the WKContentRuleList JSON for a NetworkAccessPolicy. Returns an empty
// string when no restriction is required (nothing needs installing).
//
// Ordering matters: WKContentRuleList evaluates rules in order and
// "ignore-previous-rules" cancels the actions of preceding matching rules.
//   1. block-all remote (only when defaultAction == Block)
//   2. allow-list  -> ignore-previous-rules (only meaningful under block-all)
//   3. block-list  -> block (last, so it overrides any allow)
std::string buildContentRuleListJson(const ucf::infrastructure::webview::NetworkAccessPolicy& policy)
{
    using DefaultAction = ucf::infrastructure::webview::NetworkAccessPolicy::DefaultAction;

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

    // Blocked hosts are appended last so they win even when defaultAction==Allow.
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

// FNV-1a hash of the rule JSON, used as a stable WKContentRuleListStore
// identifier so identical policies are compiled once and reused across runs.
std::string stableRuleIdentifier(const std::string& json)
{
    std::uint64_t h = 1469598103934665603ULL;
    for (unsigned char c : json)
    {
        h ^= c;
        h *= 1099511628211ULL;
    }
    std::ostringstream oss;
    oss << "miniapp.netpolicy." << std::hex << h;
    return oss.str();
}

} // namespace


namespace {

// Serialize a JavaScript evaluation result (id) into a JSON string.
std::string jsonStringFromJSResult(id result)
{
    if (!result || [result isKindOfClass:[NSNull class]])
    {
        return "null";
    }
    // NSJSONWritingFragmentsAllowed permits scalar top-level values (macOS 10.15+).
    NSData* data = [NSJSONSerialization dataWithJSONObject:result
                                                  options:NSJSONWritingFragmentsAllowed
                                                    error:nil];
    if (data)
    {
        return std::string(static_cast<const char*>(data.bytes), data.length);
    }
    return std::string([[result description] UTF8String] ?: "");
}

} // namespace

@implementation WkWebViewHelper

- (void)webView:(WKWebView*)webView didStartProvisionalNavigation:(WKNavigation*)navigation
{
    (void)webView;
    (void)navigation;
    if (self.backend)
    {
        auto* backendPtr = static_cast<ucf::infrastructure::webview::WkWebView*>(self.backend);
        NSURL* url = webView.URL;
        std::string urlStr = url ? std::string(url.absoluteString.UTF8String) : "";
        backendPtr->platformFireNavigationStarted(urlStr);
    }
}

- (void)webView:(WKWebView*)webView didFinishNavigation:(WKNavigation*)navigation
{
    (void)webView;
    (void)navigation;
    if (self.backend)
    {
        auto* backendPtr = static_cast<ucf::infrastructure::webview::WkWebView*>(self.backend);
        backendPtr->platformFireLoadFinished(true);
    }
}

- (void)webView:(WKWebView*)webView didFailNavigation:(WKNavigation*)navigation withError:(NSError*)error
{
    (void)webView;
    (void)navigation;
    if (self.backend)
    {
        auto* backendPtr = static_cast<ucf::infrastructure::webview::WkWebView*>(self.backend);
        int code = static_cast<int>(error.code);
        std::string message = error.localizedDescription.UTF8String ?: "";
        backendPtr->platformFireLoadFailed(code, message);
        backendPtr->platformFireLoadFinished(false);
    }
}

- (void)webView:(WKWebView*)webView didFailProvisionalNavigation:(WKNavigation*)navigation withError:(NSError*)error
{
    (void)webView;
    (void)navigation;
    if (self.backend)
    {
        auto* backendPtr = static_cast<ucf::infrastructure::webview::WkWebView*>(self.backend);
        int code = static_cast<int>(error.code);
        std::string message = error.localizedDescription.UTF8String ?: "";
        backendPtr->platformFireLoadFailed(code, message);
        backendPtr->platformFireLoadFinished(false);
    }
}

- (void)webView:(WKWebView*)webView didCommitNavigation:(WKNavigation*)navigation
{
    (void)webView;
    (void)navigation;
    if (self.backend && webView.URL)
    {
        auto* backendPtr = static_cast<ucf::infrastructure::webview::WkWebView*>(self.backend);
        std::string url = std::string(webView.URL.absoluteString.UTF8String);
        backendPtr->platformFireUrlChanged(url);
    }
}

- (WKWebView*)webView:(WKWebView*)webView
    createWebViewWithConfiguration:(WKWebViewConfiguration*)configuration
               forNavigationAction:(WKNavigationAction*)navigationAction
                    windowFeatures:(WKWindowFeatures*)windowFeatures
{
    (void)configuration;
    (void)windowFeatures;
    // WKWebView has no window/tab manager of its own, so it delegates the
    // "open a new window" decision to the host. Without this, target=_blank /
    // window.open requests (common in third-party SSO logins such as WeChat or
    // Google) would be silently dropped and appear as a dead button. As an
    // embedded container we have no separate window, so load the request into
    // the current web view instead of creating a new one.
    if (!navigationAction.targetFrame)
    {
        [webView loadRequest:navigationAction.request];
    }
    return nil;
}

- (void)observeValueForKeyPath:(NSString*)keyPath
                      ofObject:(id)object
                        change:(NSDictionary*)change
                       context:(void*)context
{
    (void)context;
    (void)change;
    if ([keyPath isEqualToString:@"title"])
    {
        WKWebView* webView = (WKWebView*)object;
        if (self.backend && webView.title)
        {
            auto* backendPtr = static_cast<ucf::infrastructure::webview::WkWebView*>(self.backend);
            std::string title = std::string(webView.title.UTF8String);
            backendPtr->platformFireTitleChanged(title);
        }
    }
}

- (void)userContentController:(WKUserContentController*)userContentController
      didReceiveScriptMessage:(WKScriptMessage*)message
{
    (void)userContentController;
    if (self.backend && message.body && [message.body isKindOfClass:[NSString class]])
    {
        auto* backendPtr = static_cast<ucf::infrastructure::webview::WkWebView*>(self.backend);
        std::string channel = std::string(message.name.UTF8String ?: "");
        std::string payload = std::string(((NSString*)message.body).UTF8String ?: "");
        backendPtr->platformFireScriptMessage(channel, payload);
    }
}

- (void)webView:(WKWebView*)webView startURLSchemeTask:(id<WKURLSchemeTask>)urlSchemeTask
{
    (void)webView;
    if (!self.backend)
    {
        NSError* error = [NSError errorWithDomain:@"WebViewEngine" code:500 userInfo:nil];
        [urlSchemeTask didFailWithError:error];
        return;
    }

    NSURLRequest* nsRequest = urlSchemeTask.request;
    if (!nsRequest)
    {
        NSError* error = [NSError errorWithDomain:@"WebViewEngine" code:400 userInfo:nil];
        [urlSchemeTask didFailWithError:error];
        return;
    }

    // Convert NSURLRequest to WebRequest
    std::string url = nsRequest.URL ? std::string(nsRequest.URL.absoluteString.UTF8String) : "";
    std::string method = nsRequest.HTTPMethod ? std::string(nsRequest.HTTPMethod.UTF8String) : "GET";

    ucf::infrastructure::webview::WebRequest webRequest;
    webRequest.url = url;
    webRequest.method = method;

    // Extract headers
    if (nsRequest.allHTTPHeaderFields)
    {
        for (NSString* key in nsRequest.allHTTPHeaderFields)
        {
            NSString* value = nsRequest.allHTTPHeaderFields[key];
            webRequest.headers[std::string(key.UTF8String)] = std::string(value.UTF8String ?: "");
        }
    }

    // Extract body as vector<uint8_t>
    if (nsRequest.HTTPBody)
    {
        const uint8_t* bodyPtr = static_cast<const uint8_t*>(nsRequest.HTTPBody.bytes);
        webRequest.body.assign(bodyPtr, bodyPtr + nsRequest.HTTPBody.length);
    }

    // Call dispatcher
    auto* backendPtr = static_cast<ucf::infrastructure::webview::WkWebView*>(self.backend);
    ucf::infrastructure::webview::InterceptResult result = backendPtr->internalDispatcher().dispatch(webRequest);

    // Handle result
    using ucf::infrastructure::webview::InterceptAction;
    switch (result.action)
    {
        case InterceptAction::Continue:
        {
            // This scheme handler is installed only for the registered custom
            // schemes. Native http(s) requests bypass this handler and are
            // processed by WebKit. For a custom scheme, Continue is unsupported
            // because WebKit delegates the full response responsibility to the
            // custom handler.
            NSString* scheme = urlSchemeTask.request.URL.scheme ?: @"?";
            NSString* desc = [NSString stringWithFormat:@"%@:// request was not handled", scheme];
            NSError* error = [NSError errorWithDomain:@"WebViewEngine"
                                                 code:501
                                             userInfo:@{ NSLocalizedDescriptionKey: desc }];
            [urlSchemeTask didFailWithError:error];
            break;
        }
        case InterceptAction::Respond:
        {
            if (!result.response)
            {
                // Respond without a response is a programming error at the
                // interceptor level. Fail the task explicitly instead of
                // leaving it pending.
                NSError* error = [NSError errorWithDomain:@"WebViewEngine"
                                                     code:500
                                                 userInfo:@{ NSLocalizedDescriptionKey:
                                                                 @"Respond action with no response" }];
                [urlSchemeTask didFailWithError:error];
                break;
            }
            {
                const auto& response = *result.response;

                // Build headers dictionary
                NSMutableDictionary* headerDict = [NSMutableDictionary dictionary];
                for (const auto& [key, value] : response.headers)
                {
                    headerDict[[NSString stringWithUTF8String:key.c_str()]] =
                        [NSString stringWithUTF8String:value.c_str()];
                }
                if (!response.mimeType.empty())
                {
                    headerDict[@"Content-Type"] = [NSString stringWithUTF8String:response.mimeType.c_str()];
                }

                // Create response
                NSHTTPURLResponse* nsResponse = [[NSHTTPURLResponse alloc]
                    initWithURL:urlSchemeTask.request.URL
                    statusCode:response.statusCode
                    HTTPHeaderFields:headerDict
                ];

                // Send response and data
                [urlSchemeTask didReceiveResponse:nsResponse];
                if (!response.body.empty())
                {
                    NSData* bodyData = [NSData dataWithBytes:response.body.data()
                                                     length:response.body.size()];
                    [urlSchemeTask didReceiveData:bodyData];
                }
                [urlSchemeTask didFinish];
                [nsResponse release];
            }
            break;
        }
        case InterceptAction::Fail:
        {
            int errorCode = result.error ? result.error->code : 500;
            NSString* errorMsg = result.error ?
                [NSString stringWithUTF8String:result.error->message.c_str()] :
                @"Unknown error";
            NSDictionary* userInfo = @{ NSLocalizedDescriptionKey : errorMsg };
            NSError* error = [NSError errorWithDomain:@"WebViewEngine" code:errorCode userInfo:userInfo];
            [urlSchemeTask didFailWithError:error];
            break;
        }
    }
}

- (void)webView:(WKWebView*)webView stopURLSchemeTask:(id<WKURLSchemeTask>)urlSchemeTask
{
    (void)webView;
    (void)urlSchemeTask;
}

@end

namespace ucf::infrastructure::webview {

struct WkWebView::Impl
{
    WKWebView* webView = nullptr;
    WkWebViewHelper* helper = nullptr;
    InterceptorDispatcher dispatcher;
    std::vector<std::string> customSchemes;
    bool initialized = false;
    bool ready = false;
};

WkWebView::WkWebView()
    : m_impl(std::make_unique<Impl>())
{
}

WkWebView::~WkWebView()
{
    @autoreleasepool {
        // Tear down in the correct order: detach delegates and remove the KVO
        // observer while the helper is still alive, then release the web view,
        // and only after that release the helper. Releasing the helper first
        // could leave a dangling observer registered on the web view.
        if (m_impl->webView)
        {
            m_impl->webView.navigationDelegate = nil;
            m_impl->webView.UIDelegate = nil;
            [m_impl->webView removeObserver:m_impl->helper forKeyPath:@"title"];
            [m_impl->webView release];
        }
        if (m_impl->helper)
        {
            m_impl->helper.backend = nullptr;
            [m_impl->helper release];
        }
    }
}

bool WkWebView::initialize(const WebViewInitOptions& options)
{
    if (m_impl->initialized)
    {
        return false;
    }

    // Deduplicate while preserving order; fall back to the default "app" scheme
    // when none are provided. Every scheme must be a valid, non-reserved custom
    // scheme, otherwise WebKit would throw when registering the handler.
    std::vector<std::string> schemes;
    for (const std::string& scheme : options.customSchemes)
    {
        if (!isValidCustomScheme(scheme))
        {
            return false;
        }
        if (std::find(schemes.cbegin(), schemes.cend(), scheme) == schemes.cend())
        {
            schemes.push_back(scheme);
        }
    }
    if (schemes.empty())
    {
        schemes.emplace_back("app");
    }

    @autoreleasepool {
        m_impl->initialized = true;

        m_impl->helper = [[WkWebViewHelper alloc] init];
        m_impl->helper.backend = reinterpret_cast<void*>(this);

        WKWebViewConfiguration* config = [[[WKWebViewConfiguration alloc] init] autorelease];
        WKUserContentController* ucc = [[[WKUserContentController alloc] init] autorelease];

        // Add document-start scripts
        for (const auto& script : options.documentStartScripts)
        {
            NSString* scriptStr = [NSString stringWithUTF8String:script.c_str()];
            WKUserScript* userScript = [[[WKUserScript alloc] initWithSource:scriptStr
                                                                injectionTime:WKUserScriptInjectionTimeAtDocumentStart
                                                             forMainFrameOnly:YES] autorelease];
            [ucc addUserScript:userScript];
        }

        // Register script message handlers
        for (const auto& channel : options.scriptChannels)
        {
            NSString* channelName = [NSString stringWithUTF8String:channel.c_str()];
            [ucc addScriptMessageHandler:m_impl->helper name:channelName];
        }

        config.userContentController = ucc;

        // Controls whether JavaScript may open windows without a user gesture.
        config.preferences.javaScriptCanOpenWindowsAutomatically = options.allowPopups ? YES : NO;

        // Custom scheme handlers MUST be attached to the configuration BEFORE
        // the WKWebView is constructed; changes to config after construction
        // are silently ignored by WebKit.
        m_impl->customSchemes = schemes;
        for (const std::string& scheme : schemes)
        {
            NSString* nsScheme = [NSString stringWithUTF8String:scheme.c_str()];
            [config setURLSchemeHandler:m_impl->helper forURLScheme:nsScheme];
        }

        m_impl->webView = [[WKWebView alloc] initWithFrame:NSZeroRect configuration:config];
        m_impl->webView.navigationDelegate = m_impl->helper;
        m_impl->webView.UIDelegate = m_impl->helper;

        // customUserAgent replaces the full User-Agent string. (applicationNameForUserAgent
        // would only append the app name to the default UA, which is not an override.)
        if (options.userAgentOverride.has_value() && !options.userAgentOverride->empty())
        {
            NSString* ua = [NSString stringWithUTF8String:options.userAgentOverride->c_str()];
            m_impl->webView.customUserAgent = ua;
        }

        // Observe title changes
        [m_impl->webView addObserver:m_impl->helper
                          forKeyPath:@"title"
                             options:NSKeyValueObservingOptionNew
                             context:nullptr];

        // Build the network access-control rules (if any). When a policy is
        // present we DEFER readiness until the rules are compiled and installed,
        // so the very first navigation is already governed by them (fail-closed).
        std::string ruleJson;
        if (options.networkPolicy.has_value())
        {
            ruleJson = buildContentRuleListJson(*options.networkPolicy);
        }

        // The block retains the helper; the destructor nulls helper.backend
        // before releasing it, so a destroyed backend is detected and skipped.
        WkWebViewHelper* helper = m_impl->helper;

        if (ruleJson.empty())
        {
            m_impl->ready = true;

            // Notify readiness on the next main-loop turn so that consumers which
            // register their callback right after initialize() still receive it,
            // and so the contract matches asynchronous backends (e.g. WebView2).
            dispatch_async(dispatch_get_main_queue(), ^{
                if (helper.backend)
                {
                    auto* backendPtr = static_cast<ucf::infrastructure::webview::WkWebView*>(helper.backend);
                    backendPtr->platformFireWebViewReady();
                }
            });
        }
        else
        {
            // Fail-closed: stay not-ready until the rule list is installed.
            const std::string ruleId = stableRuleIdentifier(ruleJson);
            NSString* identifier = [NSString stringWithUTF8String:ruleId.c_str()];
            NSString* encoded = [NSString stringWithUTF8String:ruleJson.c_str()];
            WKContentRuleListStore* store = [WKContentRuleListStore defaultStore];
            WKUserContentController* uccRef = ucc;

            void (^installAndReady)(WKContentRuleList*) = ^(WKContentRuleList* list) {
                if (!helper.backend) { return; }
                [uccRef addContentRuleList:list];
                auto* backendPtr = static_cast<ucf::infrastructure::webview::WkWebView*>(helper.backend);
                backendPtr->m_impl->ready = true;
                backendPtr->platformFireWebViewReady();
            };

            // Reuse a previously compiled list when the policy is unchanged;
            // otherwise compile it. Both callbacks are delivered on the main thread.
            [store lookUpContentRuleListForIdentifier:identifier
                                    completionHandler:^(WKContentRuleList* found, NSError* lookupError) {
                (void)lookupError;
                if (found)
                {
                    installAndReady(found);
                    return;
                }
                [store compileContentRuleListForIdentifier:identifier
                                    encodedContentRuleList:encoded
                                         completionHandler:^(WKContentRuleList* compiled, NSError* compileError) {
                    if (compiled)
                    {
                        installAndReady(compiled);
                    }
                    else if (helper.backend)
                    {
                        auto* backendPtr = static_cast<ucf::infrastructure::webview::WkWebView*>(helper.backend);
                        const char* desc = compileError.localizedDescription.UTF8String;
                        std::string msg = desc ? std::string(desc) : std::string("content rule list compilation failed");
                        backendPtr->platformFireLoadFailed(kWebErrorContentPolicyCompileFailed, msg);
                    }
                }];
            }];
        }
    }

    return true;
}

bool WkWebView::isReady() const
{
    return m_impl->ready;
}

void WkWebView::loadUrl(const std::string& url)
{
    if (!m_impl->webView || url.empty())
    {
        return;
    }

    @autoreleasepool {
        NSString* urlStr = [NSString stringWithUTF8String:url.c_str()];
        NSURL* nsUrl = [NSURL URLWithString:urlStr];
        if (nsUrl)
        {
            NSURLRequest* request = [NSURLRequest requestWithURL:nsUrl];
            [m_impl->webView loadRequest:request];
        }
    }
}

void WkWebView::reload()
{
    if (m_impl->webView)
    {
        [m_impl->webView reload];
    }
}

void WkWebView::stop()
{
    if (m_impl->webView)
    {
        [m_impl->webView stopLoading];
    }
}

void WkWebView::evaluateJavaScript(const std::string& js, JavaScriptResultCallback callback)
{
    if (!m_impl->webView || js.empty())
    {
        if (callback)
        {
            callback(false, "", WebError{ -1, "web view not ready or empty script" });
        }
        return;
    }

    @autoreleasepool {
        NSString* jsStr = [NSString stringWithUTF8String:js.c_str()];
        if (!callback)
        {
            [m_impl->webView evaluateJavaScript:jsStr completionHandler:nil];
            return;
        }
        // The block copies the std::function by value (block copy invokes the
        // C++ copy constructor), so the callback outlives this scope.
        [m_impl->webView evaluateJavaScript:jsStr completionHandler:^(id result, NSError* error) {
            if (error)
            {
                callback(false, "",
                         WebError{ static_cast<int>(error.code),
                                   std::string(error.localizedDescription.UTF8String ?: "") });
                return;
            }
            callback(true, jsonStringFromJSResult(result), WebError{});
        }];
    }
}

InterceptorId WkWebView::addRequestInterceptor(std::shared_ptr<IRequestInterceptor> interceptor)
{
    return m_impl->dispatcher.add(interceptor);
}

void WkWebView::removeRequestInterceptor(InterceptorId id)
{
    m_impl->dispatcher.remove(id);
}

void WkWebView::clearRequestInterceptors()
{
    m_impl->dispatcher.clear();
}

InterceptorDispatcher& WkWebView::internalDispatcher()
{
    return m_impl->dispatcher;
}

NativeHostHandle WkWebView::nativeHostHandle() const
{
    return reinterpret_cast<NativeHostHandle>(m_impl->webView);
}

} // namespace ucf::infrastructure::webview

#else

// Stub for non-macOS platforms (should use WebView2 on Windows, Stub elsewhere)
namespace ucf::infrastructure::webview {

WkWebView::WkWebView() : m_impl(std::make_unique<Impl>()) {}
WkWebView::~WkWebView() = default;
bool WkWebView::initialize(const WebViewInitOptions&) { return false; }
bool WkWebView::isReady() const { return false; }
void WkWebView::loadUrl(const std::string&) {}
void WkWebView::reload() {}
void WkWebView::stop() {}
void WkWebView::evaluateJavaScript(const std::string&, JavaScriptResultCallback) {}
InterceptorId WkWebView::addRequestInterceptor(std::shared_ptr<IRequestInterceptor>) { return 0; }
void WkWebView::removeRequestInterceptor(InterceptorId) {}
void WkWebView::clearRequestInterceptors() {}
NativeHostHandle WkWebView::nativeHostHandle() const { return 0; }

} // namespace ucf::infrastructure::webview

#endif
