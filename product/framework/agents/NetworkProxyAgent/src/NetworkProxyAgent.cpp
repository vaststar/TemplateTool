#include "NetworkProxyAgent.h"
#include "NetworkProxyAgentLogger.h"
#include "AddonLocator.h"
#include "AddonProtocol.h"

#include <ucf/Utilities/SystemUtils/SystemProxyUtils.h>
#include <ucf/Utilities/SystemUtils/CertStoreUtils.h>

#include <magic_enum/magic_enum.hpp>

#include <vector>

namespace ucf::agents {

// ════════════════════════════════════════════════════════════
//  State machine
// ════════════════════════════════════════════════════════════

bool NetworkProxyAgent::tryTransition(AgentState to)
{
    switch (to)
    {
    case AgentState::Starting:
        if (casFrom({AgentState::Idle, AgentState::Terminated}, to))
        {
            return true;
        }
        break;
    case AgentState::Running:
        if (casFrom({AgentState::Starting}, to))
        {
            return true;
        }
        break;
    case AgentState::Stopping:
        if (casFrom({AgentState::Running, AgentState::Starting}, to))
        {
            return true;
        }
        break;
    case AgentState::Terminated:
        if (casFrom({AgentState::Stopping, AgentState::Starting, AgentState::Running}, to))
        {
            return true;
        }
        break;
    case AgentState::Idle:
        break;  // Idle is the initial state, no transition back
    }

    NPA_LOG_DEBUG("Transition to " << magic_enum::enum_name(to)
                 << " rejected, current=" << magic_enum::enum_name(state()));
    return false;
}

bool NetworkProxyAgent::casFrom(std::initializer_list<AgentState> fromStates,
                                AgentState to)
{
    for (auto expected : fromStates)
    {
        if (mState.compare_exchange_strong(expected, to,
                                           std::memory_order_acq_rel,
                                           std::memory_order_acquire))
        {
            NPA_LOG_DEBUG("State: " << magic_enum::enum_name(expected)
                         << " -> " << magic_enum::enum_name(to));
            return true;
        }
    }
    return false;
}

bool NetworkProxyAgent::beginShutdown(ShutdownReason reason)
{
    if (!tryTransition(AgentState::Stopping))
    {
        NPA_LOG_DEBUG("Shutdown skipped, reason=" << magic_enum::enum_name(reason)
                      << ", state=" << magic_enum::enum_name(state()));
        return false;
    }

    NPA_LOG_INFO("Shutdown begin, reason=" << magic_enum::enum_name(reason));
    fireNotification(&INetworkProxyAgentCallback::onAgentStateChanged,
                     AgentState::Stopping);
    return true;
}

void NetworkProxyAgent::finalizeShutdown(ShutdownReason reason,
                                         int exitCode,
                                         bool crashed)
{
    // Sync barrier — ensures start() has finished writing mConfig
    {
        std::lock_guard<std::mutex> lock(mLifecycleMutex);
    }

    // Unified cleanup for TcpChannel / ProcessBridge / buffer.
    doCleanup();

    if (mConfig.autoSystemProxy)
    {
        ucf::utilities::SystemProxyUtils::disableHttpProxy();
    }

    if (!tryTransition(AgentState::Terminated))
    {
        NPA_LOG_DEBUG("Finalize shutdown skipped terminal transition, reason="
                      << magic_enum::enum_name(reason)
                      << ", state=" << magic_enum::enum_name(state()));
        return;
    }

    fireNotification(&INetworkProxyAgentCallback::onAgentStateChanged,
                     AgentState::Terminated);

    switch (reason)
    {
    case ShutdownReason::UserRequest:
        fireNotification(&INetworkProxyAgentCallback::onStatusMessage,
                         std::string("Proxy stopped"));
        break;

    case ShutdownReason::TcpDisconnected:
        fireNotification(&INetworkProxyAgentCallback::onAddonDisconnected);
        fireNotification(&INetworkProxyAgentCallback::onStatusMessage,
                         std::string("Addon disconnected"));
        break;

    case ShutdownReason::ProcessExited:
        fireNotification(&INetworkProxyAgentCallback::onAddonDisconnected);
        fireNotification(&INetworkProxyAgentCallback::onStatusMessage,
                         std::string("Proxy process exited (code ")
                             + std::to_string(exitCode)
                             + ", crashed="
                             + (crashed ? "true" : "false")
                             + ")");
        break;
    }
}

// ════════════════════════════════════════════════════════════
//  Factory
// ════════════════════════════════════════════════════════════

std::shared_ptr<INetworkProxyAgent> INetworkProxyAgent::create()
{
    return std::make_shared<NetworkProxyAgent>();
}

// ════════════════════════════════════════════════════════════
//  Lifecycle
// ════════════════════════════════════════════════════════════

NetworkProxyAgent::NetworkProxyAgent()
{
    mProcessBridge = ucf::utilities::IProcessBridge::create();
    mTcpChannel = ucf::utilities::ITcpChannel::create();
    NPA_LOG_DEBUG("NetworkProxyAgent created");
}

NetworkProxyAgent::~NetworkProxyAgent()
{
    stop();
    NPA_LOG_DEBUG("NetworkProxyAgent destroyed");
}

// ════════════════════════════════════════════════════════════
//  Start / Stop
// ════════════════════════════════════════════════════════════

bool NetworkProxyAgent::start(const AgentConfig& config)
{
    if (!tryTransition(AgentState::Starting))
    {
        NPA_LOG_WARN("start() rejected, state=" << magic_enum::enum_name(state()));
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(mLifecycleMutex);
        mConfig = config;
    }

    fireNotification(&INetworkProxyAgentCallback::onAgentStateChanged,
                     AgentState::Starting);
    fireNotification(&INetworkProxyAgentCallback::onStatusMessage,
                     std::string("Starting proxy on port ")
                         + std::to_string(config.proxyPort) + "...");

    // Step 1: Locate addon executable
    std::string addonPath = detail::AddonLocator::findAddonExecutable();
    if (addonPath.empty())
    {
        NPA_LOG_ERROR("Addon executable not found");
        failStart("Proxy addon executable not found");
        return false;
    }

    // Step 2: Start TCP control channel on ephemeral port
    ucf::utilities::TcpChannelConfig tcpConfig;
    tcpConfig.listenAddress = "127.0.0.1";
    tcpConfig.listenPort = 0;   // OS picks
    tcpConfig.maxConnections = 1;

    mTcpChannel->registerCallback(shared_from_this());

    if (!mTcpChannel->startListening(tcpConfig))
    {
        NPA_LOG_ERROR("Failed to start TCP control channel");
        failStart("Failed to start control channel");
        return false;
    }

    int controlPort = mTcpChannel->listeningPort();
    NPA_LOG_INFO("Control channel listening on port " << controlPort);

    // Step 3: Launch addon process
    ucf::utilities::ProcessBridgeConfig pbConfig;
    pbConfig.executablePath = addonPath;
    pbConfig.arguments = {
        "--proxy-port", std::to_string(config.proxyPort),
        "--control-port", std::to_string(controlPort)
    };
    pbConfig.captureStdout = true;
    pbConfig.captureStderr = true;
    pbConfig.stopTimeoutMs = config.stopTimeoutMs;

    mProcessBridge->registerCallback(shared_from_this());

    if (!mProcessBridge->start(pbConfig))
    {
        NPA_LOG_ERROR("Failed to start addon process");
        failStart("Failed to start proxy addon process");
        return false;
    }

    // Remain in Starting state until addon connects via TCP (onClientConnected)
    fireNotification(&INetworkProxyAgentCallback::onStatusMessage,
                     std::string("Process started, waiting for addon connection..."));
    return true;
}

void NetworkProxyAgent::stop()
{
    if (!beginShutdown(ShutdownReason::UserRequest))
    {
        return;
    }

    finalizeShutdown(ShutdownReason::UserRequest);
}

void NetworkProxyAgent::failStart(const std::string& errorMessage)
{
    doCleanup();
    tryTransition(AgentState::Terminated);
    fireNotification(&INetworkProxyAgentCallback::onAgentStateChanged,
                     AgentState::Terminated);
    fireNotification(&INetworkProxyAgentCallback::onError, errorMessage);
}

void NetworkProxyAgent::doCleanup()
{
    if (mTcpChannel)
    {
        mTcpChannel->stop();
    }
    if (mProcessBridge)
    {
        mProcessBridge->stop();
    }
    {
        std::lock_guard<std::mutex> lock(mBufferMutex);
        mTcpBuffer.clear();
    }
}

// ════════════════════════════════════════════════════════════
//  State query
// ════════════════════════════════════════════════════════════

AgentState NetworkProxyAgent::state() const
{
    return mState.load(std::memory_order_acquire);
}

// ════════════════════════════════════════════════════════════
//  Commands to addon
// ════════════════════════════════════════════════════════════

void NetworkProxyAgent::sendCommand(const std::string& jsonCommand)
{
    if (!mTcpChannel || !mTcpChannel->isConnected())
    {
        return;
    }
    // Ensure newline-delimited
    std::string msg = jsonCommand;
    if (msg.empty() || msg.back() != '\n')
    {
        msg += '\n';
    }
    mTcpChannel->send(msg);
}

void NetworkProxyAgent::setInterceptEnabled(bool enabled)
{
    sendCommand(detail::AddonProtocol::buildSetIntercept(enabled));
}

void NetworkProxyAgent::resumeRequest(const std::string& flowId)
{
    sendCommand(detail::AddonProtocol::buildResumeFlow(flowId));
}

void NetworkProxyAgent::dropRequest(const std::string& flowId)
{
    sendCommand(detail::AddonProtocol::buildDropFlow(flowId));
}

void NetworkProxyAgent::updateRules(const std::string& ruleType,
                                     const std::string& rulesJson)
{
    sendCommand(detail::AddonProtocol::buildUpdateRules(ruleType, rulesJson));
}

// ════════════════════════════════════════════════════════════
//  Certificate management
// ════════════════════════════════════════════════════════════

CertTrustStatus NetworkProxyAgent::certTrustStatus() const
{
    return mCertStatus.load();
}

std::string NetworkProxyAgent::caCertPath() const
{
    return detail::AddonLocator::findCACertPath();
}

CertInstallResult NetworkProxyAgent::installCACert()
{
    std::string path = caCertPath();
    auto result = ucf::utilities::CertStoreUtils::installToTrustStore(path);

    if (result == CertInstallResult::Success)
    {
        mCertStatus = CertTrustStatus::Trusted;
    }

    return result;
}

void NetworkProxyAgent::checkCertStatus()
{
    std::string path = caCertPath();
    auto trustStatus = ucf::utilities::CertStoreUtils::checkTrustStatus(path);
    mCertStatus = trustStatus;
}

// ════════════════════════════════════════════════════════════
//  IProcessBridgeCallback
// ════════════════════════════════════════════════════════════

void NetworkProxyAgent::onProcessStarted(int64_t pid)
{
    NPA_LOG_INFO("Addon process started, PID=" << pid);
    fireNotification(&INetworkProxyAgentCallback::onStatusMessage,
                     std::string("Addon process started (PID=")
                         + std::to_string(pid) + ")");
}

void NetworkProxyAgent::onProcessStopped(int exitCode, bool crashed)
{
    NPA_LOG_INFO("Addon process exited, code=" << exitCode
                 << ", crashed=" << crashed);

    if (!beginShutdown(ShutdownReason::ProcessExited))
    {
        return;
    }

    finalizeShutdown(ShutdownReason::ProcessExited, exitCode, crashed);
}

void NetworkProxyAgent::onProcessError(const std::string& errorMessage)
{
    NPA_LOG_ERROR("Addon process error: " << errorMessage);
    fireNotification(&INetworkProxyAgentCallback::onError, errorMessage);
}

void NetworkProxyAgent::onStdout(const std::string& data)
{
    NPA_LOG_DEBUG("[PROXY-STDOUT] " << data);
}

void NetworkProxyAgent::onStderr(const std::string& data)
{
    NPA_LOG_DEBUG("[PROXY-STDERR] " << data);
}

// ════════════════════════════════════════════════════════════
//  ITcpChannelCallback
// ════════════════════════════════════════════════════════════

void NetworkProxyAgent::onClientConnected()
{
    NPA_LOG_INFO("Addon connected via TCP");

    if (tryTransition(AgentState::Running))
    {
        fireNotification(&INetworkProxyAgentCallback::onAgentStateChanged,
                         AgentState::Running);

        // Enable system proxy now that addon is fully connected
        if (mConfig.autoSystemProxy)
        {
            ucf::utilities::SystemProxyUtils::enableHttpProxy("127.0.0.1", mConfig.proxyPort);
        }

        fireNotification(&INetworkProxyAgentCallback::onAddonConnected);
        fireNotification(&INetworkProxyAgentCallback::onStatusMessage,
                         std::string("Addon connected, proxy running on port ")
                             + std::to_string(mConfig.proxyPort));
    }
    else
    {
        NPA_LOG_WARN("Addon connected but agent is shutting down, ignoring");
    }
}

void NetworkProxyAgent::onClientDisconnected()
{
    NPA_LOG_INFO("Addon TCP connection lost");

    if (!beginShutdown(ShutdownReason::TcpDisconnected))
    {
        return;
    }

    finalizeShutdown(ShutdownReason::TcpDisconnected);
}

void NetworkProxyAgent::onDataReceived(const std::string& data)
{
    std::vector<std::string> lines;
    {
        std::lock_guard<std::mutex> lock(mBufferMutex);
        mTcpBuffer += data;

        // Parse newline-delimited JSON messages
        while (true)
        {
            auto pos = mTcpBuffer.find('\n');
            if (pos == std::string::npos)
            {
                break;
            }

            lines.push_back(mTcpBuffer.substr(0, pos));
            mTcpBuffer.erase(0, pos + 1);
        }
    }

    for (auto& line : lines)
    {
        if (!line.empty())
        {
            handleAddonMessage(line);
        }
    }
}

void NetworkProxyAgent::onError(const std::string& errorMessage)
{
    NPA_LOG_ERROR("TCP channel error: " << errorMessage);
    fireNotification(&INetworkProxyAgentCallback::onError, errorMessage);
}

// ════════════════════════════════════════════════════════════
//  Message handling
// ════════════════════════════════════════════════════════════

void NetworkProxyAgent::handleAddonMessage(const std::string& jsonLine)
{
    auto msg = detail::AddonProtocol::parseMessage(jsonLine);

    if (msg.type.empty())
    {
        NPA_LOG_WARN("Received message without type field");
        return;
    }

    if (msg.type == "request")
    {
        fireNotification(&INetworkProxyAgentCallback::onRequestCaptured,
                         msg.flowId, msg.rawJson);
    }
    else if (msg.type == "response")
    {
        fireNotification(&INetworkProxyAgentCallback::onResponseCaptured,
                         msg.flowId, msg.rawJson);
    }
    else if (msg.type == "intercepted")
    {
        fireNotification(&INetworkProxyAgentCallback::onRequestIntercepted,
                         msg.flowId, msg.rawJson);
    }
    else if (msg.type == "error")
    {
        fireNotification(&INetworkProxyAgentCallback::onError,
                         std::string("Addon error: ") + msg.message);
    }
    else if (msg.type == "status")
    {
        fireNotification(&INetworkProxyAgentCallback::onStatusMessage,
                         msg.message);
    }
    else
    {
        NPA_LOG_WARN("Unknown addon message type: " << msg.type);
    }
}

} // namespace ucf::agents
