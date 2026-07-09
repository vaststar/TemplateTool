#include "MiniAppSession.h"

#include <QWindow>

#include <memory>

#include <ucf/Agents/MiniAppRuntimeAgent/IMiniAppRuntimeAgentCallback.h>
#include <ucf/Agents/MiniAppRuntimeAgent/MiniAppRuntimeAgentFactory.h>

#include "MiniAppSdk.h"
#include "jsapi/SystemApiModule.h"

#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

namespace MiniAppRuntime {

// Bridges agent lifecycle notifications into UIView logging. The UI shell does
// not need to react to these, so it only records them for diagnostics.
class MiniAppSession::Callback : public ucf::agents::IMiniAppRuntimeAgentCallback
{
public:
    explicit Callback(std::string appId) : m_appId(std::move(appId)) {}

    void onLoadFinished(bool ok) override
    {
        UIVIEW_LOG_INFO("MiniAppSession[" << m_appId << "] load finished ok=" << ok);
    }

    void onLoadFailed(int errorCode, const std::string& errorString) override
    {
        UIVIEW_LOG_WARN("MiniAppSession[" << m_appId << "] load failed code=" << errorCode
                                          << " msg=" << errorString);
    }

private:
    std::string m_appId;
};

MiniAppSession::MiniAppSession(MiniAppContext context)
    : m_context(std::move(context))
    , m_agent(ucf::agents::createMiniAppRuntimeAgent())
    , m_callback(std::make_shared<Callback>(m_context.id.toStdString()))
{
}

MiniAppSession::~MiniAppSession()
{
    if (m_agent)
    {
        m_agent->shutdown();
    }
}

QWindow* MiniAppSession::nativeWindow() const
{
    if (!m_agent)
    {
        return nullptr;
    }
    const ucf::infrastructure::webview::NativeHostHandle handle = m_agent->nativeHostHandle();
    if (handle == 0)
    {
        return nullptr;
    }
    return QWindow::fromWinId(static_cast<WId>(handle));
}

void MiniAppSession::start()
{
    if (m_started || !m_agent)
    {
        return;
    }
    m_started = true;

    ucf::agents::MiniAppRuntimeAgentConfig config;
    config.appId = m_context.id.toStdString();
    config.packageDir = m_context.packageDir.toStdString();
    if (!m_context.entry.isEmpty())
    {
        config.entry = m_context.entry.toStdString();
    }
    for (const QString& permission : m_context.permissions)
    {
        config.grantedPermissions.push_back(permission.toStdString());
    }
    config.documentStartScripts.emplace_back(miniAppSdkJs());

    m_agent->registerCallback(m_callback);
    m_agent->registerBridgeHandler(
        std::make_shared<SystemApiModule>(m_context.id.toStdString(), m_context.name.toStdString()));

    if (!m_agent->initialize(config))
    {
        UIVIEW_LOG_WARN("MiniAppSession initialize failed id=" << m_context.id.toStdString());
        return;
    }

    UIVIEW_LOG_INFO("MiniAppSession start id=" << m_context.id.toStdString()
                                               << " entry=" << config.entry);
    m_agent->loadEntry();
}

} // namespace MiniAppRuntime
