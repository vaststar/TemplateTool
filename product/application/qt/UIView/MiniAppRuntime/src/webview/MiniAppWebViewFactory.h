#pragma once

class QObject;

namespace MiniAppRuntime {

class IMiniAppWebView;

// Creates the web-view backend for the current platform (WKWebView on macOS,
// stub elsewhere). Ownership transfers to the caller, or to `parent` if set.
IMiniAppWebView* createMiniAppWebView(QObject* parent = nullptr);

} // namespace MiniAppRuntime
