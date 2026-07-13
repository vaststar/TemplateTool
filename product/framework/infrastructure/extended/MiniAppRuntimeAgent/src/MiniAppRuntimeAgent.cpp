#include "MiniAppRuntimeAgent.h"

#include <algorithm>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "RuntimeRequestInterceptor.h"
#include "RuntimeResourceResolver.h"
#include "MiniAppSdk.h"
#include <ucf/Agents/MiniAppRuntimeAgent/MiniAppRuntimeAgentFactory.h>
#include <ucf/Infrastructure/WebViewEngine/WebViewFactory.h>
#include <ucf/Utilities/JsonUtils/JsonValue.h>

namespace ucf::agents {
namespace {

std::string trimLeadingSlashes(const std::string& input)
{
    size_t pos = 0;
    while (pos < input.size() && input[pos] == '/')
    {
        ++pos;
    }
    return input.substr(pos);
}

std::string wrapDispatch(const std::string& json)
{
    const std::string jsLiteral = ucf::utilities::JsonValue(json).dump();
    return "if(window.__miniapp){window.__miniapp._dispatch(JSON.parse(" + jsLiteral + "));}";
}

} // namespace

class RuntimeWebViewCallback : public ucf::infrastructure::webview::IWebViewCallback
{
public:
    explicit RuntimeWebViewCallback(MiniAppRuntimeAgent* owner)
        : m_owner(owner)
    {
    }

    void onUrlChanged(const std::string& url) override;
    void onTitleChanged(const std::string& title) override;
    void onLoadFinished(bool success) override;
    void onLoadFailed(int errorCode, const std::string& errorMsg) override;
    void onScriptMessage(const std::string& channel, const std::string& payload) override;
    void onWebViewReady() override;

private:
    MiniAppRuntimeAgent* m_owner = nullptr; // not owned
};

class MiniAppRuntimeAgent::Impl
{
public:
    MiniAppRuntimeAgentConfig config;
    std::shared_ptr<ucf::infrastructure::webview::IWebView> webView;
    std::shared_ptr<RuntimeWebViewCallback> callback;
    std::shared_ptr<RuntimeRequestInterceptor> runtimeInterceptor;
    ucf::infrastructure::webview::InterceptorId runtimeInterceptorId = 0;
    MiniAppBridgeCore bridgeCore;
    std::mutex mutex;
    bool initialized = false;
    bool ready = false;
    bool pendingLoadEntry = false;
};

void RuntimeWebViewCallback::onUrlChanged(const std::string& url)
{
    if (m_owner)
    {
        m_owner->onWebViewUrlChanged(url);
    }
}

void RuntimeWebViewCallback::onTitleChanged(const std::string& title)
{
    if (m_owner)
    {
        m_owner->onWebViewTitleChanged(title);
    }
}

void RuntimeWebViewCallback::onLoadFinished(bool success)
{
    if (m_owner)
    {
        m_owner->onWebViewLoadFinished(success);
    }
}

void RuntimeWebViewCallback::onLoadFailed(int errorCode, const std::string& errorMsg)
{
    if (m_owner)
    {
        m_owner->onWebViewLoadFailed(errorCode, errorMsg);
    }
}

void RuntimeWebViewCallback::onScriptMessage(const std::string& channel, const std::string& payload)
{
    if (m_owner)
    {
        m_owner->onWebViewScriptMessage(channel, payload);
    }
}

void RuntimeWebViewCallback::onWebViewReady()
{
    if (m_owner)
    {
        m_owner->onWebViewReady();
    }
}

MiniAppRuntimeAgent::MiniAppRuntimeAgent()
    : m_impl(std::make_unique<Impl>())
{
}

MiniAppRuntimeAgent::~MiniAppRuntimeAgent()
{
    shutdown();
}

bool MiniAppRuntimeAgent::initialize(const MiniAppRuntimeAgentConfig& config)
{
    bool fireReady = false;
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        if (m_impl->initialized)
        {
            return false;
        }

        m_impl->config = config;
        m_impl->webView = ucf::infrastructure::webview::createWebView();
        if (!m_impl->webView)
        {
            return false;
        }

        ucf::infrastructure::webview::WebViewInitOptions options;
        if (!config.appScheme.empty())
        {
            options.customSchemes.push_back(config.appScheme);
        }
        options.userDataFolder = config.userDataFolder;
        options.documentStartScripts.emplace_back(miniAppSdkJs());
        options.documentStartScripts.insert(options.documentStartScripts.end(),
                                            config.documentStartScripts.begin(),
                                            config.documentStartScripts.end());

        options.scriptChannels.push_back(config.bridgeChannel);

        // Engine-level network access control. Semantics mirror the former host
        // allow-list: a non-empty allow-list is exclusive; an empty list falls
        // back to allowNetworkByDefault. Only remote protocols are affected, and
        // the engine defers readiness until the rules are installed (fail-closed).
        if (!config.allowedNetworkHosts.empty() || !config.allowNetworkByDefault)
        {
            ucf::infrastructure::webview::NetworkAccessPolicy policy;
            policy.defaultAction = ucf::infrastructure::webview::NetworkAccessPolicy::DefaultAction::Block;
            policy.allowedHosts = config.allowedNetworkHosts;
            options.networkPolicy = std::move(policy);
        }

        if (!m_impl->webView->initialize(options))
        {
            return false;
        }

        m_impl->callback = std::make_shared<RuntimeWebViewCallback>(this);
        m_impl->webView->registerCallback(m_impl->callback);

        auto resolver = std::make_shared<RuntimeResourceResolver>(config.appId, config.packageDir, config.appScheme);
        m_impl->runtimeInterceptor = std::make_shared<RuntimeRequestInterceptor>(resolver);
        m_impl->runtimeInterceptorId = m_impl->webView->addRequestInterceptor(m_impl->runtimeInterceptor);

        // Deliver outbound bridge messages into the page. The sink captures a
        // web-view copy so replies fired while the agent lock is held do not
        // re-enter the agent's mutex.
        auto webViewForSink = m_impl->webView;
        m_impl->bridgeCore.setMessageSink([webViewForSink](const std::string& json) {
            if (webViewForSink)
            {
                webViewForSink->evaluateJavaScript(wrapDispatch(json));
            }
        });
        m_impl->bridgeCore.setGrantedPermissions(config.grantedPermissions);

        m_impl->initialized = true;

        // Synchronous back-ends (e.g. the current WkWebView / stub) are already
        // ready here; async back-ends will drive readiness via onWebViewReady().
        if (m_impl->webView->isReady())
        {
            m_impl->ready = true;
            fireReady = true;
        }
    }

    if (fireReady)
    {
        fireNotification(&IMiniAppRuntimeAgentCallback::onReadyChanged, true);
    }
    return true;
}

void MiniAppRuntimeAgent::shutdown()
{
    bool fireNotReady = false;
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        if (!m_impl->initialized)
        {
            return;
        }

        if (m_impl->webView && m_impl->callback)
        {
            m_impl->webView->unRegisterCallback(m_impl->callback);
        }
        if (m_impl->webView && m_impl->runtimeInterceptorId != 0)
        {
            m_impl->webView->removeRequestInterceptor(m_impl->runtimeInterceptorId);
            m_impl->runtimeInterceptorId = 0;
        }

        m_impl->bridgeCore.clearHandlers();
        m_impl->bridgeCore.setMessageSink(nullptr);
        m_impl->bridgeCore.setGrantedPermissions({});

        m_impl->runtimeInterceptor.reset();
        m_impl->callback.reset();
        m_impl->webView.reset();

        fireNotReady = m_impl->ready;
        m_impl->ready = false;
        m_impl->initialized = false;
        m_impl->pendingLoadEntry = false;
    }

    if (fireNotReady)
    {
        fireNotification(&IMiniAppRuntimeAgentCallback::onReadyChanged, false);
    }
}

bool MiniAppRuntimeAgent::isReady() const
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->ready;
}

void MiniAppRuntimeAgent::loadEntry()
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    if (!m_impl->webView)
    {
        return;
    }

    // The engine may still be installing the network access-control rules
    // (fail-closed). Defer the load until onWebViewReady() fires so the first
    // navigation is already governed by them.
    if (!m_impl->ready)
    {
        m_impl->pendingLoadEntry = true;
        return;
    }

    m_impl->webView->loadUrl(buildEntryUrlLocked());
}

std::string MiniAppRuntimeAgent::buildEntryUrlLocked() const
{
    const std::string& entry = m_impl->config.entry.empty() ? std::string("index.html") : m_impl->config.entry;
    if (entry.find("://") != std::string::npos)
    {
        return entry;
    }

    const std::string scheme = m_impl->config.appScheme.empty() ? std::string("app") : m_impl->config.appScheme;
    return scheme + "://" + m_impl->config.appId + "/" + trimLeadingSlashes(entry);
}

void MiniAppRuntimeAgent::postEvent(const std::string& eventName, const JsonValue& data)
{
    // Route through the bridge core: it owns the outbound sink and produces a
    // well-formed {type:"event"} envelope, and has its own mutex so we avoid
    // re-entering the agent lock.
    m_impl->bridgeCore.postEvent(eventName, data);
}

void MiniAppRuntimeAgent::registerBridgeHandler(std::shared_ptr<IBridgeMethodHandler> handler)
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->bridgeCore.registerHandler(std::move(handler));
}

ucf::infrastructure::webview::NativeHostHandle MiniAppRuntimeAgent::nativeHostHandle() const
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    if (!m_impl->webView)
    {
        return 0;
    }
    return m_impl->webView->nativeHostHandle();
}

void MiniAppRuntimeAgent::onWebViewReady()
{
    bool fireReady = false;
    bool doPendingLoad = false;
    std::string pendingUrl;
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        if (m_impl->initialized && !m_impl->ready)
        {
            m_impl->ready = true;
            fireReady = true;

            if (m_impl->pendingLoadEntry && m_impl->webView)
            {
                m_impl->pendingLoadEntry = false;
                pendingUrl = buildEntryUrlLocked();
                doPendingLoad = true;
            }
        }
    }
    if (fireReady)
    {
        fireNotification(&IMiniAppRuntimeAgentCallback::onReadyChanged, true);
    }
    if (doPendingLoad)
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        if (m_impl->webView)
        {
            m_impl->webView->loadUrl(pendingUrl);
        }
    }
}

void MiniAppRuntimeAgent::onWebViewUrlChanged(const std::string& url)
{
    fireNotification(&IMiniAppRuntimeAgentCallback::onUrlChanged, url);
}

void MiniAppRuntimeAgent::onWebViewTitleChanged(const std::string& title)
{
    fireNotification(&IMiniAppRuntimeAgentCallback::onTitleChanged, title);
}

void MiniAppRuntimeAgent::onWebViewLoadFinished(bool success)
{
    fireNotification(&IMiniAppRuntimeAgentCallback::onLoadFinished, success);
}

void MiniAppRuntimeAgent::onWebViewLoadFailed(int errorCode, const std::string& errorMessage)
{
    fireNotification(&IMiniAppRuntimeAgentCallback::onLoadFailed, errorCode, errorMessage);
}

void MiniAppRuntimeAgent::onWebViewScriptMessage(const std::string& channel, const std::string& payload)
{
    bool isBridge = false;
    {
        std::lock_guard<std::mutex> lock(m_impl->mutex);
        isBridge = (channel == m_impl->config.bridgeChannel);
    }
    // bridgeCore has its own mutex; dispatch outside the agent lock so a handler
    // that synchronously calls back into the agent does not deadlock.
    if (isBridge)
    {
        m_impl->bridgeCore.onInboundMessage(payload);
    }
}

std::shared_ptr<IMiniAppRuntimeAgent> createMiniAppRuntimeAgent()
{
    return std::make_shared<MiniAppRuntimeAgent>();
}

} // namespace ucf::agents
