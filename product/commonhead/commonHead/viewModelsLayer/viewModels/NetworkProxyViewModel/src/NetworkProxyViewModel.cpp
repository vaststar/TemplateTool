#include "NetworkProxyViewModel.h"

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

namespace commonHead::viewModels {

// ════════════════════════════════════════════════════════════
//  Factory
// ════════════════════════════════════════════════════════════

std::shared_ptr<INetworkProxyViewModel> INetworkProxyViewModel::createInstance(
    commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
{
    return std::make_shared<NetworkProxyViewModel>(commonHeadFramework);
}

// ════════════════════════════════════════════════════════════
//  Lifecycle
// ════════════════════════════════════════════════════════════

NetworkProxyViewModel::NetworkProxyViewModel(commonHead::ICommonHeadFrameworkWptr commonHeadFramework)
    : INetworkProxyViewModel(commonHeadFramework)
{
    COMMONHEAD_LOG_DEBUG("create NetworkProxyViewModel");
}

NetworkProxyViewModel::~NetworkProxyViewModel()
{
    stopProxy();
    COMMONHEAD_LOG_DEBUG("destroy NetworkProxyViewModel");
}

std::string NetworkProxyViewModel::getViewModelName() const
{
    return "NetworkProxyViewModel";
}

void NetworkProxyViewModel::init()
{
    mAgent = ucf::utilities::INetworkProxyAgent::create();
    mAgent->registerCallback(shared_from_this());
}

// ════════════════════════════════════════════════════════════
//  Start / Stop — delegate to agent
// ════════════════════════════════════════════════════════════

void NetworkProxyViewModel::startProxy(const model::ProxyConfig& config)
{
    ucf::utilities::AgentConfig agentConfig;
    agentConfig.proxyPort = config.proxyPort;
    agentConfig.autoSystemProxy = config.autoSystemProxy;
    agentConfig.stopTimeoutMs = config.stopTimeoutMs;

    if (!mAgent->start(agentConfig))
    {
        COMMONHEAD_LOG_WARN("startProxy() — agent rejected start");
    }
}

void NetworkProxyViewModel::stopProxy()
{
    if (mAgent)
    {
        mAgent->stop();
    }
}

model::ProxyState NetworkProxyViewModel::proxyState() const
{
    if (!mAgent)
    {
        return model::ProxyState::Idle;
    }
    return model::toProxyState(mAgent->state());
}

// ════════════════════════════════════════════════════════════
//  Commands — delegate to agent
// ════════════════════════════════════════════════════════════

void NetworkProxyViewModel::sendCommand(const std::string& jsonCommand)
{
    if (mAgent) { mAgent->sendCommand(jsonCommand); }
}

void NetworkProxyViewModel::setInterceptEnabled(bool enabled)
{
    if (mAgent) { mAgent->setInterceptEnabled(enabled); }
}

void NetworkProxyViewModel::resumeRequest(const std::string& flowId)
{
    if (mAgent) { mAgent->resumeRequest(flowId); }
}

void NetworkProxyViewModel::dropRequest(const std::string& flowId)
{
    if (mAgent) { mAgent->dropRequest(flowId); }
}

void NetworkProxyViewModel::updateRules(const std::string& ruleType,
                                         const std::string& rulesJson)
{
    if (mAgent) { mAgent->updateRules(ruleType, rulesJson); }
}

// ════════════════════════════════════════════════════════════
//  Certificate management — delegate to agent
// ════════════════════════════════════════════════════════════

model::CertStatus NetworkProxyViewModel::certStatus() const
{
    if (!mAgent)
    {
        return model::CertStatus::Unknown;
    }
    return mAgent->certTrustStatus();
}

std::string NetworkProxyViewModel::caCertPath() const
{
    if (!mAgent)
    {
        return {};
    }
    return mAgent->caCertPath();
}

void NetworkProxyViewModel::installCACert()
{
    if (!mAgent)
    {
        return;
    }

    auto result = mAgent->installCACert();

    switch (result)
    {
    case model::CertInstallResult::Success:
        fireNotification(&INetworkProxyViewModelCallback::onCertStatusChanged,
                         model::CertStatus::Trusted);
        fireNotification(&INetworkProxyViewModelCallback::onStatusMessage,
                         std::string("CA certificate installed successfully"));
        break;
    case model::CertInstallResult::UserCancelled:
        fireNotification(&INetworkProxyViewModelCallback::onStatusMessage,
                         std::string("Certificate installation cancelled by user"));
        break;
    case model::CertInstallResult::FileNotFound:
        fireNotification(&INetworkProxyViewModelCallback::onError,
                         std::string("CA certificate file not found: ") + caCertPath());
        break;
    case model::CertInstallResult::ParseError:
        fireNotification(&INetworkProxyViewModelCallback::onError,
                         std::string("Failed to parse CA certificate file"));
        break;
    default:
        fireNotification(&INetworkProxyViewModelCallback::onError,
                         std::string("Failed to install CA certificate"));
        break;
    }
}

void NetworkProxyViewModel::checkCertStatus()
{
    if (!mAgent)
    {
        return;
    }
    mAgent->checkCertStatus();
    auto status = mAgent->certTrustStatus();
    fireNotification(&INetworkProxyViewModelCallback::onCertStatusChanged, status);
}

// ════════════════════════════════════════════════════════════
//  INetworkProxyAgentCallback — translate to ViewModel callbacks
// ════════════════════════════════════════════════════════════

void NetworkProxyViewModel::onAgentStateChanged(ucf::utilities::AgentState state)
{
    auto proxyState = model::toProxyState(state);
    fireNotification(&INetworkProxyViewModelCallback::onProxyStateChanged, proxyState);
}

void NetworkProxyViewModel::onAddonConnected()
{
    fireNotification(&INetworkProxyViewModelCallback::onAddonConnectionChanged, true);
}

void NetworkProxyViewModel::onAddonDisconnected()
{
    fireNotification(&INetworkProxyViewModelCallback::onAddonConnectionChanged, false);
}

void NetworkProxyViewModel::onRequestCaptured(const std::string& flowId,
                                               const std::string& rawJson)
{
    fireNotification(&INetworkProxyViewModelCallback::onRequestCaptured, flowId, rawJson);
}

void NetworkProxyViewModel::onResponseCaptured(const std::string& flowId,
                                                const std::string& rawJson)
{
    fireNotification(&INetworkProxyViewModelCallback::onResponseCaptured, flowId, rawJson);
}

void NetworkProxyViewModel::onRequestIntercepted(const std::string& flowId,
                                                  const std::string& detailJson)
{
    fireNotification(&INetworkProxyViewModelCallback::onRequestIntercepted,
                     flowId, detailJson);
}

void NetworkProxyViewModel::onStatusMessage(const std::string& message)
{
    fireNotification(&INetworkProxyViewModelCallback::onStatusMessage, message);
}

void NetworkProxyViewModel::onError(const std::string& errorMessage)
{
    fireNotification(&INetworkProxyViewModelCallback::onError, errorMessage);
}

} // namespace commonHead::viewModels

