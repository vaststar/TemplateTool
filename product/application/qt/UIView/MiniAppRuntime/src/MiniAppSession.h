#pragma once

#include <QObject>

#include <memory>

#include "MiniAppRuntime/MiniAppContext.h"

namespace MiniAppRuntime {

class IMiniAppWebView;
class AppSchemeHandler;
class PermissionGate;
class MiniAppBridge;

// Owns the runtime for one running mini-app: web-view backend, scheme handler,
// permission gate and JS bridge. start() wires them up and navigates to the entry.
class MiniAppSession : public QObject
{
    Q_OBJECT
public:
    explicit MiniAppSession(MiniAppContext context, QObject* parent = nullptr);
    ~MiniAppSession() override;

    void start();

    // The web view for embedding into a host window. Valid after construction.
    [[nodiscard]] IMiniAppWebView* webView() const;

    [[nodiscard]] const MiniAppContext& context() const { return m_context; }

private:
    MiniAppContext m_context;
    IMiniAppWebView* m_webView = nullptr; // child QObject, parented to this
    std::unique_ptr<AppSchemeHandler> m_schemeHandler;
    std::unique_ptr<PermissionGate> m_permissionGate;
    std::unique_ptr<MiniAppBridge> m_bridge;
    bool m_started = false;
};

} // namespace MiniAppRuntime
