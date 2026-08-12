#pragma once

#include <memory>

#include <ucf/infrastructure/WebViewEngine/WebViewEngineExport.h>

namespace ucf::infrastructure::webview {

class IWebView;

[[nodiscard]] WEB_VIEW_ENGINE_API std::shared_ptr<IWebView> createWebView();

} // namespace ucf::infrastructure::webview
