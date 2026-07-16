#pragma once

#include <string>

#include <ucf/Infrastructure/WebViewEngine/WebViewTypes.h>

namespace ucf::infrastructure::webview::content_policy {

// Build the declarative content-filter JSON for a NetworkAccessPolicy. The rule
// schema is identical for macOS WKContentRuleList and Linux
// WebKitUserContentFilter, so both backends share this single implementation.
// Returns an empty string when the policy imposes no restriction (nothing needs
// to be installed).
//
// Ordering matters: the engine evaluates rules in order and
// "ignore-previous-rules" cancels the actions of preceding matching rules.
//   1. block-all remote (only when defaultAction == Block)
//   2. allow-list  -> ignore-previous-rules (only meaningful under block-all)
//   3. block-list  -> block (last, so it overrides any allow)
[[nodiscard]] std::string buildContentRuleListJson(const NetworkAccessPolicy& policy);

// FNV-1a hash of the rule JSON rendered as lowercase hex. Backends prepend their
// own store-specific prefix to form a stable rule-list identifier, so identical
// policies compile once and are reused across runs.
[[nodiscard]] std::string ruleListHashHex(const std::string& json);

} // namespace ucf::infrastructure::webview::content_policy
