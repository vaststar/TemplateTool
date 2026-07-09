#pragma once

#include <string>

namespace ucf::agents {

class IMiniAppRuntimeAgentCallback
{
public:
    virtual ~IMiniAppRuntimeAgentCallback() = default;

    virtual void onReadyChanged(bool ready) {}
    virtual void onLoadFinished(bool success) {}
    virtual void onLoadFailed(int errorCode, const std::string& errorMessage) {}
    virtual void onTitleChanged(const std::string& title) {}
    virtual void onUrlChanged(const std::string& url) {}
    virtual void onRawScriptMessage(const std::string& channel, const std::string& payload) {}
};

} // namespace ucf::agents
