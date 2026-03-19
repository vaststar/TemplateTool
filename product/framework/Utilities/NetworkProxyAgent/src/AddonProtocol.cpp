#include "AddonProtocol.h"
#include "NetworkProxyAgentLogger.h"

#include <ucf/Utilities/JsonUtils/JsonValue.h>

namespace ucf::utilities::detail {

// ═══════════════════════════════════════════════════════════════
//  Incoming message parsing
// ═══════════════════════════════════════════════════════════════

AddonMessage AddonProtocol::parseMessage(const std::string& jsonLine)
{
    AddonMessage result;
    result.rawJson = jsonLine;

    auto parsed = JsonValue::parse(jsonLine);
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

    return result;
}

// ═══════════════════════════════════════════════════════════════
//  Outgoing command builders
// ═══════════════════════════════════════════════════════════════

std::string AddonProtocol::buildSetIntercept(bool enabled)
{
    auto obj = JsonValue::object();
    obj.set("type", "set_intercept");
    obj.set("enabled", enabled);
    return obj.dump() + "\n";
}

std::string AddonProtocol::buildResumeFlow(const std::string& flowId)
{
    auto obj = JsonValue::object();
    obj.set("type", "resume_flow");
    obj.set("flow_id", flowId);
    return obj.dump() + "\n";
}

std::string AddonProtocol::buildDropFlow(const std::string& flowId)
{
    auto obj = JsonValue::object();
    obj.set("type", "drop_flow");
    obj.set("flow_id", flowId);
    return obj.dump() + "\n";
}

std::string AddonProtocol::buildUpdateRules(const std::string& ruleType,
                                             const std::string& rulesJson)
{
    // The rules are already a JSON array string; we parse and embed it
    auto rulesVal = JsonValue::parse(rulesJson);

    auto obj = JsonValue::object();
    obj.set("type", "update_" + ruleType);

    if (rulesVal.isArray())
    {
        obj.set("rules", std::move(rulesVal));
    }
    else
    {
        // If parsing failed, embed as empty array and log warning
        NPA_LOG_WARN("Failed to parse rules JSON for " << ruleType << ", using empty array");
        obj.set("rules", JsonValue::array());
    }

    return obj.dump() + "\n";
}

std::string AddonProtocol::buildSetThrottle(bool enabled,
                                             int downloadKbps,
                                             int uploadKbps)
{
    auto obj = JsonValue::object();
    obj.set("type", "set_throttle");
    obj.set("enabled", enabled);
    obj.set("download_kbps", downloadKbps);
    obj.set("upload_kbps", uploadKbps);
    return obj.dump() + "\n";
}

} // namespace ucf::utilities::detail
