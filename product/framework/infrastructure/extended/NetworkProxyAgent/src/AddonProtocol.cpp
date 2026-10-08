#include "AddonProtocol.h"
#include "NetworkProxyAgentLogger.h"

#include <ucf/utilities/JsonUtils/JsonValue.h>

#include <algorithm>
#include <cstddef>

namespace ucf::agents::detail {

namespace {

bool isConfigRevision(const std::string& value)
{
    constexpr std::size_t maxDigits = 20;
    constexpr const char* maxRevision = "18446744073709551615";
    return !value.empty() && value.size() <= maxDigits
        && (value.size() == 1 || value.front() != '0')
        && std::all_of(value.begin(), value.end(), [](char digit) {
            return digit >= '0' && digit <= '9';
        })
        && (value.size() < maxDigits || value <= maxRevision);
}

} // namespace

// ═══════════════════════════════════════════════════════════════
//  Incoming message parsing
// ═══════════════════════════════════════════════════════════════

AddonMessage AddonProtocol::parseMessage(const std::string& jsonLine)
{
    AddonMessage result;
    result.rawJson = jsonLine;

    auto parsed = ucf::utilities::JsonValue::parse(jsonLine);
    if (!parsed.isObject())
    {
        NPA_LOG_WARN("Failed to parse addon message as JSON object");
        return result;
    }

    // Extract "type"
    auto typeVal = parsed.get("type");
    if (typeVal.isString())
    {
        result.type = typeVal.asString().value_or("");
    }

    // Extract "flow_id"
    auto flowIdVal = parsed.get("flow_id");
    if (flowIdVal.isString())
    {
        result.flowId = flowIdVal.asString().value_or("");
    }

    // Extract "message" (used by status and error types)
    auto messageVal = parsed.get("message");
    if (messageVal.isString())
    {
        result.message = messageVal.asString().value_or("");
    }

    if (result.type == "proxy_config_result")
    {
        const auto sessionIdVal = parsed.get("session_id");
        const auto revisionVal = parsed.get("revision");
        const auto acceptedVal = parsed.get("accepted");
        if (!sessionIdVal.isString() || !revisionVal.isString()
            || !acceptedVal.isBool() || !messageVal.isString())
        {
            NPA_LOG_WARN("Invalid proxy configuration reply fields");
            result.type.clear();
            return result;
        }
        result.sessionId = sessionIdVal.asString().value_or("");
        result.revision = revisionVal.asString().value_or("");
        if (result.sessionId.empty() || !isConfigRevision(result.revision))
        {
            NPA_LOG_WARN("Invalid proxy configuration reply session or revision");
            result.type.clear();
            return result;
        }
        result.accepted = acceptedVal.asBool().value_or(false);
    }

    // Extract "reason" (used by intercept_finished)
    auto reasonVal = parsed.get("reason");
    if (reasonVal.isString())
    {
        result.reason = reasonVal.asString().value_or("");
    }

    return result;
}

// ═══════════════════════════════════════════════════════════════
//  Outgoing command builders
// ═══════════════════════════════════════════════════════════════

std::string AddonProtocol::buildSetIntercept(bool enabled)
{
    auto obj = ucf::utilities::JsonValue::object();
    obj.set("type", "set_intercept");
    obj.set("enabled", enabled);
    return obj.dump() + "\n";
}

std::string AddonProtocol::buildResumeFlow(const std::string& flowId)
{
    auto obj = ucf::utilities::JsonValue::object();
    obj.set("type", "resume_flow");
    obj.set("flow_id", flowId);
    return obj.dump() + "\n";
}

std::string AddonProtocol::buildDropFlow(const std::string& flowId)
{
    auto obj = ucf::utilities::JsonValue::object();
    obj.set("type", "drop_flow");
    obj.set("flow_id", flowId);
    return obj.dump() + "\n";
}

std::string AddonProtocol::buildUpdateRules(const std::string& ruleType,
                                             const std::string& rulesJson)
{
    // The rules are already a JSON array string; we parse and embed it
    auto rulesVal = ucf::utilities::JsonValue::parse(rulesJson);

    auto obj = ucf::utilities::JsonValue::object();
    obj.set("type", "update_" + ruleType);

    if (rulesVal.isArray())
    {
        obj.set("rules", std::move(rulesVal));
    }
    else
    {
        // If parsing failed, embed as empty array and log warning
        NPA_LOG_WARN("Failed to parse rules JSON for " << ruleType << ", using empty array");
        obj.set("rules", ucf::utilities::JsonValue::array());
    }

    return obj.dump() + "\n";
}

std::string AddonProtocol::buildSetThrottle(bool enabled,
                                             int downloadKbps,
                                             int uploadKbps)
{
    auto obj = ucf::utilities::JsonValue::object();
    obj.set("type", "set_throttle");
    obj.set("enabled", enabled);
    obj.set("download_kbps", downloadKbps);
    obj.set("upload_kbps", uploadKbps);
    return obj.dump() + "\n";
}

} // namespace ucf::agents::detail
