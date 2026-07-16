#include "WebViewContentPolicy.h"

#include <cstdint>
#include <sstream>
#include <vector>

namespace ucf::infrastructure::webview::content_policy {

namespace {

// Escape a host so it can be embedded literally inside a content-filter
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

// JSON-escape a string (only backslash and double-quote need handling here, as
// regex fragments contain no control characters).
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
        tail += "([a-z0-9-]+\\.)*";
    }
    tail += escapeHostForRegex(host);
    tail += "(:[0-9]+)?(/|$)";
    return tail;
}

} // namespace

std::string buildContentRuleListJson(const NetworkAccessPolicy& policy)
{
    using DefaultAction = NetworkAccessPolicy::DefaultAction;

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

std::string ruleListHashHex(const std::string& json)
{
    std::uint64_t h = 1469598103934665603ULL;
    for (unsigned char c : json)
    {
        h ^= c;
        h *= 1099511628211ULL;
    }
    std::ostringstream oss;
    oss << std::hex << h;
    return oss.str();
}

} // namespace ucf::infrastructure::webview::content_policy
