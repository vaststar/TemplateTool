#include "webview/WkWebViewBackend.h"

#if defined(Q_OS_MACOS) || defined(__APPLE__)

#import <AppKit/AppKit.h>
#import <WebKit/WebKit.h>

#include <QUrl>
#include <QWindow>

#include "AppSchemeHandler.h"

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

// ---------------------------------------------------------------------------
// Objective-C bridge: one instance per backend, acting as the app:// scheme
// handler, the "miniapp" script-message handler and the navigation delegate.
// Forwards to the owning C++ backend via a raw, non-owning back-pointer.
// ---------------------------------------------------------------------------
@interface MiniAppWkHelper : NSObject <WKURLSchemeHandler, WKScriptMessageHandler, WKNavigationDelegate>
@property (nonatomic, assign) MiniAppRuntime::WkWebViewBackend* backend;
@end

@implementation MiniAppWkHelper

// --- WKURLSchemeHandler: serve app:// from the package directory ---
- (void)webView:(WKWebView*)webView startURLSchemeTask:(id<WKURLSchemeTask>)urlSchemeTask
{
    (void)webView;
    MiniAppRuntime::AppSchemeHandler* handler = self.backend ? self.backend->schemeHandler() : nullptr;
    NSURL* nsUrl = urlSchemeTask.request.URL;
    const MiniAppRuntime::AppSchemeHandler::Response resp =
        handler ? handler->resolve(QUrl::fromNSURL(nsUrl)) : MiniAppRuntime::AppSchemeHandler::Response{};

    if (!resp.ok)
    {
        NSError* error = [NSError errorWithDomain:@"MiniAppRuntime"
                                             code:resp.status
                                         userInfo:nil];
        [urlSchemeTask didFailWithError:error];
        return;
    }

    NSData* data = [NSData dataWithBytes:resp.data.constData() length:static_cast<NSUInteger>(resp.data.size())];
    NSDictionary* headers = @{
        @"Content-Type": resp.mimeType.toNSString(),
        @"Content-Length": [NSString stringWithFormat:@"%lu", (unsigned long)data.length],
        @"Access-Control-Allow-Origin": @"*"
    };
    NSHTTPURLResponse* response = [[[NSHTTPURLResponse alloc] initWithURL:nsUrl
                                                              statusCode:200
                                                             HTTPVersion:@"HTTP/1.1"
                                                            headerFields:headers] autorelease];
    [urlSchemeTask didReceiveResponse:response];
    [urlSchemeTask didReceiveData:data];
    [urlSchemeTask didFinish];
}

- (void)webView:(WKWebView*)webView stopURLSchemeTask:(id<WKURLSchemeTask>)urlSchemeTask
{
    (void)webView;
    (void)urlSchemeTask;
    // Nothing to cancel: responses are produced synchronously above.
}

// --- WKScriptMessageHandler: page -> native ---
- (void)userContentController:(WKUserContentController*)userContentController
      didReceiveScriptMessage:(WKScriptMessage*)message
{
    (void)userContentController;
    if (!self.backend)
    {
        return;
    }
    if ([message.body isKindOfClass:[NSString class]])
    {
        self.backend->notifyMessage(QString::fromNSString((NSString*)message.body));
    }
}

// --- WKNavigationDelegate: load lifecycle ---
- (void)webView:(WKWebView*)webView didFinishNavigation:(WKNavigation*)navigation
{
    (void)webView;
    (void)navigation;
    if (self.backend) { self.backend->notifyLoadFinished(true); }
}

- (void)webView:(WKWebView*)webView didFailNavigation:(WKNavigation*)navigation withError:(NSError*)error
{
    (void)webView;
    (void)navigation;
    (void)error;
    if (self.backend) { self.backend->notifyLoadFinished(false); }
}

- (void)webView:(WKWebView*)webView didFailProvisionalNavigation:(WKNavigation*)navigation withError:(NSError*)error
{
    (void)webView;
    (void)navigation;
    (void)error;
    if (self.backend) { self.backend->notifyLoadFinished(false); }
}

@end

namespace MiniAppRuntime {

struct WkWebViewBackend::Impl
{
    WKWebView* webView = nullptr;              // owned (retain/release, MRC)
    MiniAppWkHelper* helper = nullptr;         // owned (retain/release, MRC)
    QWindow* foreignWindow = nullptr;          // wraps the WKWebView's NSView
    bool created = false;
};

WkWebViewBackend::WkWebViewBackend(QObject* parent)
    : IMiniAppWebView(parent)
    , m_impl(std::make_unique<Impl>())
{
}

WkWebViewBackend::~WkWebViewBackend()
{
    if (m_impl->helper)
    {
        m_impl->helper.backend = nullptr;
        [m_impl->helper release];
        m_impl->helper = nil;
    }
    if (m_impl->webView)
    {
        m_impl->webView.navigationDelegate = nil;
        [m_impl->webView release];
        m_impl->webView = nil;
    }
    // m_impl->foreignWindow is owned by the QWidget container once embedded.
}

void WkWebViewBackend::loadUrl(const QUrl& url)
{
    @autoreleasepool {
        if (!m_impl->created)
        {
            m_impl->created = true;

            m_impl->helper = [[MiniAppWkHelper alloc] init];
            m_impl->helper.backend = this;

            WKWebViewConfiguration* config = [[[WKWebViewConfiguration alloc] init] autorelease];
            if (!scheme().isEmpty())
            {
                [config setURLSchemeHandler:m_impl->helper forURLScheme:scheme().toNSString()];
            }

            WKUserContentController* ucc = [[[WKUserContentController alloc] init] autorelease];
            for (const QString& script : pendingScripts())
            {
                WKUserScript* userScript =
                    [[[WKUserScript alloc] initWithSource:script.toNSString()
                                            injectionTime:WKUserScriptInjectionTimeAtDocumentStart
                                         forMainFrameOnly:YES] autorelease];
                [ucc addUserScript:userScript];
            }
            [ucc addScriptMessageHandler:m_impl->helper name:@"miniapp"];
            config.userContentController = ucc;

            m_impl->webView = [[WKWebView alloc] initWithFrame:NSZeroRect configuration:config];
            m_impl->webView.navigationDelegate = m_impl->helper;

            m_impl->foreignWindow =
                QWindow::fromWinId(reinterpret_cast<WId>(m_impl->webView));

            UIVIEW_LOG_INFO("WkWebViewBackend created WKWebView scheme="
                            << scheme().toStdString()
                            << " scripts=" << pendingScripts().size());
        }

        NSURLRequest* request = [NSURLRequest requestWithURL:url.toNSURL()];
        [m_impl->webView loadRequest:request];
    }
}

void WkWebViewBackend::evaluateJavaScript(const QString& js)
{
    if (!m_impl->webView)
    {
        return;
    }
    @autoreleasepool {
        [m_impl->webView evaluateJavaScript:js.toNSString() completionHandler:nil];
    }
}

QWindow* WkWebViewBackend::asQWindow()
{
    return m_impl->foreignWindow;
}

void WkWebViewBackend::notifyMessage(const QString& body)
{
    emit messageReceived(body);
}

void WkWebViewBackend::notifyLoadFinished(bool ok)
{
    emit loadFinished(ok);
}

} // namespace MiniAppRuntime

#endif // Q_OS_MACOS
