#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include <ucf/services/InvocationService/StartupContext.h>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service::impl{

class InvocationManager final
{
public:
    InvocationManager(ucf::framework::ICoreFrameworkWPtr coreFramework);
    ~InvocationManager();
    InvocationManager(const InvocationManager&) = delete;
    InvocationManager(InvocationManager&&) = delete;
    InvocationManager& operator=(const InvocationManager&) = delete;
    InvocationManager& operator=(InvocationManager&&) = delete;
public:
    void processStartupParameters(StartupContext context);
    [[nodiscard]] std::optional<StartupContext> getStartupContext() const;
    void processCommandMessage(const std::string& message);
private:
    const ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;

    mutable std::mutex mStartupContextMutex;
    std::optional<StartupContext> mStartupContext;

    mutable std::mutex mCommandMessagesMutex;
    std::vector<std::string> mCommandMessages;//history command messages
};
}
