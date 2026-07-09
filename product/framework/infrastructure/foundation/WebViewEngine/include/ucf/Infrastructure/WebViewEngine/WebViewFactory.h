#pragma once

#include <memory>

#include <ucf/Infrastructure/InfrastructureCommonFile/InfrastructureExport.h>

namespace ucf::infrastructure::webview {

class IWebView;

[[nodiscard]] Infrastructure_EXPORT std::shared_ptr<IWebView> createWebView();

} // namespace ucf::infrastructure::webview
