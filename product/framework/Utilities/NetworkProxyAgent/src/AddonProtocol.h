#pragma once

#include <string>

namespace ucf::utilities::detail {

/// Parsed addon message from the TCP control channel.
struct AddonMessage
{
    std::string type;       ///< "request", "response", "intercepted", "status", "error"
    std::string flowId;     ///< flow_id field (empty for status/error messages)
    std::string rawJson;    ///< The original JSON line (preserved for forwarding)
    std::string message;    ///< Extracted "message" field (for status/error types)
};

/// Protocol helpers for communicating with the proxy addon.
///
/// Incoming: parses newline-delimited JSON from the addon (via TcpChannel).
/// Outgoing: builds JSON commands to send to the addon.
///
/// Uses JsonUtils/JsonValue for parsing; builds outgoing JSON with JsonValue too.
class AddonProtocol
{
public:
    // ── Incoming message parsing ──

    /// Parse a single JSON line from the addon into an AddonMessage.
    /// Returns a message with type="" if parsing fails.
    static AddonMessage parseMessage(const std::string& jsonLine);

    // ── Outgoing command builders ──
    // All return a newline-terminated JSON string ready to send via TcpChannel.

    static std::string buildSetIntercept(bool enabled);
    static std::string buildResumeFlow(const std::string& flowId);
    static std::string buildDropFlow(const std::string& flowId);
    static std::string buildUpdateRules(const std::string& ruleType,
                                        const std::string& rulesJson);
    static std::string buildSetThrottle(bool enabled,
                                        int downloadKbps = 0,
                                        int uploadKbps = 0);
};

} // namespace ucf::utilities::detail
