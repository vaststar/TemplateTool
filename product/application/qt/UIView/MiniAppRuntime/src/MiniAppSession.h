#pragma once

#include <memory>

#include <ucf/Agents/MiniAppRuntimeAgent/IMiniAppRuntimeAgent.h>

#include "MiniAppRuntime/MiniAppContext.h"

class QWindow;

namespace MiniAppRuntime {

// Owns the runtime for one running mini-app. The heavy lifting (web view,
// resource resolution, permission gating, JS bridge protocol) lives in the
// reusable MiniAppRuntimeAgent; this class only assembles the config, registers
// the app's bridge handlers and exposes the native view for embedding.
class MiniAppSession
{
public:
    explicit MiniAppSession(MiniAppContext context);
    ~MiniAppSession();

    MiniAppSession(const MiniAppSession&) = delete;
    MiniAppSession& operator=(const MiniAppSession&) = delete;

    void start();

    // Native web-view window for embedding into a host window. Valid after start().
    [[nodiscard]] QWindow* nativeWindow() const;

    [[nodiscard]] const MiniAppContext& context() const { return m_context; }

private:
    class Callback;

    MiniAppContext m_context;
    std::shared_ptr<ucf::agents::IMiniAppRuntimeAgent> m_agent;
    std::shared_ptr<Callback> m_callback;
    bool m_started = false;
};

} // namespace MiniAppRuntime
