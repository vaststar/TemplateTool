#pragma once

#include <memory>
#include <vector>
#include <mutex>

namespace ucf::framework{
    class ICoreFramework;
    using ICoreFrameworkWPtr = std::weak_ptr<ICoreFramework>;
}

namespace ucf::service{

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
    void processStartupParameters();
    void setStartupParameters(const std::vector<std::string>& args);
    std::vector<std::string> getStartupParameters() const;
    void processCommandMessage(const std::string& message);
private:
    const ucf::framework::ICoreFrameworkWPtr mCoreFrameworkWPtr;
    
    mutable std::mutex mStartupParametersMutex;
    std::vector<std::string> mStartupParameters;

    mutable std::mutex mCommandMessagesMutex;
    std::vector<std::string> mCommandMessages;//history command messages
};
}