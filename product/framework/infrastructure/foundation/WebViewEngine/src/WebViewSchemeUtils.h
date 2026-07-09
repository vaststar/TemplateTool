#pragma once

#include <optional>
#include <string>
#include <vector>

namespace ucf::infrastructure::webview::scheme_utils {

/// A valid custom scheme is a non-empty lowercase string of [a-z0-9+.-]
/// starting with [a-z], and it must not collide with a system scheme the
/// platform WebView refuses to hand off to a custom handler.
bool isValidCustomScheme(const std::string& scheme);

/// Validate, de-duplicate (preserving order), and default the requested custom
/// schemes. Falls back to a single "app" scheme when none are supplied. Returns
/// std::nullopt when any requested scheme is invalid, so callers can fail init.
std::optional<std::vector<std::string>> normalizeCustomSchemes(
    const std::vector<std::string>& requested);

} // namespace ucf::infrastructure::webview::scheme_utils
