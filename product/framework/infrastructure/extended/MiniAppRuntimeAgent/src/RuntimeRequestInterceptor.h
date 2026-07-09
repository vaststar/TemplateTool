#pragma once

#include <memory>

#include <ucf/Infrastructure/WebViewEngine/IRequestInterceptor.h>

namespace ucf::agents {

class RuntimeResourceResolver;

// One interceptor, one job: serve app:// package resources. Remote (http/https)
// network access is governed by the engine's NetworkAccessPolicy, not here.
// Assembled internally by the runtime agent; not exposed to UI.
class RuntimeRequestInterceptor : public ucf::infrastructure::webview::IRequestInterceptor
{
public:
    explicit RuntimeRequestInterceptor(std::shared_ptr<RuntimeResourceResolver> resolver);

    [[nodiscard]] int priority() const override;
    [[nodiscard]] bool match(const ucf::infrastructure::webview::WebRequest& request) const override;
    [[nodiscard]] ucf::infrastructure::webview::InterceptResult intercept(
        const ucf::infrastructure::webview::WebRequest& request) override;

private:
    std::shared_ptr<RuntimeResourceResolver> m_resolver;
};

} // namespace ucf::agents
