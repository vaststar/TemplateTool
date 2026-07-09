#pragma once

#include <ucf/Infrastructure/InfrastructureCommonFile/InfrastructureExport.h>
#include <ucf/Infrastructure/WebViewEngine/WebViewTypes.h>

namespace ucf::infrastructure::webview {

class Infrastructure_EXPORT IRequestInterceptor
{
public:
    IRequestInterceptor() = default;
    IRequestInterceptor(const IRequestInterceptor&) = delete;
    IRequestInterceptor(IRequestInterceptor&&) = delete;
    IRequestInterceptor& operator=(const IRequestInterceptor&) = delete;
    IRequestInterceptor& operator=(IRequestInterceptor&&) = delete;
    virtual ~IRequestInterceptor() = default;

public:
    [[nodiscard]] virtual int priority() const { return 0; }
    [[nodiscard]] virtual bool match(const WebRequest& request) const = 0;
    [[nodiscard]] virtual InterceptResult intercept(const WebRequest& request) = 0;
};

} // namespace ucf::infrastructure::webview
