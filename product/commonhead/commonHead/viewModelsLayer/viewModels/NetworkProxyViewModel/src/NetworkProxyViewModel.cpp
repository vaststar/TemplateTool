#include "NetworkProxyViewModel.h"

#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <ucf/Utilities/SystemUtils/CertStoreUtils.h>
#include <ucf/Utilities/NetworkProxyAgent/NetworkProxyAgentTypes.h>

namespace commonHead::viewModels {

namespace {

/// Convert low-level AgentState → ViewModel ProxyState
model::ProxyState toProxyState(ucf::utilities::AgentState s)
{
    switch (s)
    {
    case ucf::utilities::AgentState::Idle:       return model::ProxyState::Idle;
    case ucf::utilities::AgentState::Starting:   return model::ProxyState::Starting;
    case ucf::utilities::AgentState::Running:    return model::ProxyState::Running;
    case ucf::utilities::AgentState::Stopping:   return model::ProxyState::Stopping;
    case ucf::utilities::AgentState::Terminated: return model::ProxyState::Terminated;
    default:                                     return model::ProxyState::Idle;
    }
}

/// Convert low-level CertTrustStatus → ViewModel CertStatus
model::CertStatus toCertStatus(ucf::utilities::CertTrustStatus s)
{
    switch (s)
    {
    case ucf::utilities::CertTrustStatus::Unknown:      return model::CertStatus::Unknown;
    case ucf::utilities::CertTrustStatus::FileNotFound:  return model::CertStatus::FileNotFound;
    case ucf::utilities::CertTrustStatus::NotTrusted:    return model::CertStatus::NotTrusted;
    case ucf::utilities::CertTrustStatus::Trusted:       return model::CertStatus::Trusted;
    default:                                             return model::CertStatus::Unknown;
    }
}

/// Convert low-level CertInstallResult → ViewModel CertInstallResult
model::CertInstallResult toCertInstallResult(ucf::utilities::CertInstallResult r)
{
    switch (r)
    {
    case ucf::utilities::CertInstallResult::Success:       return model::CertInstallResult::Success;
    case ucf::utilities::CertInstallResult::FileNotFound:  return model::CertInstallResult::FileNotFound;
    case ucf::utilities::CertInstallResult::ParseError:    return model::CertInstallResult::ParseError;
    case ucf::utilities::CertInstallResult::UserCancelled: return model::CertInstallResult::UserCancelled;
    case ucf::utilities::CertInstallResult::Failed:        return model::CertInstallResult::Failed;
    default:                                               return model::CertInstallResult::Failed;
    }
}

} // anonymous namespace

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
    return toProxyState(mAgent->state());
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
    return toCertStatus(mAgent->certTrustStatus());
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

    auto result = toCertInstallResult(mAgent->installCACert());

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
    auto status = toCertStatus(mAgent->certTrustStatus());
    fireNotification(&INetworkProxyViewModelCallback::onCertStatusChanged, status);
}

// ════════════════════════════════════════════════════════════
//  INetworkProxyAgentCallback — translate to ViewModel callbacks
// ════════════════════════════════════════════════════════════

void NetworkProxyViewModel::onAgentStateChanged(ucf::utilities::AgentState state)
{
    auto proxyState = toProxyState(state);
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

