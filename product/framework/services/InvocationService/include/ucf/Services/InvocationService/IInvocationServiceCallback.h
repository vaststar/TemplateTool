#pragma once

#include <vector>

namespace ucf::service{

class IInvocationServiceCallback
{
public:
    IInvocationServiceCallback() = default;
    IInvocationServiceCallback(const IInvocationServiceCallback&) = delete;
    IInvocationServiceCallback(IInvocationServiceCallback&&) = delete;
    IInvocationServiceCallback& operator=(const IInvocationServiceCallback&) = delete;
    IInvocationServiceCallback& operator=(IInvocationServiceCallback&&) = delete;
    virtual ~ IInvocationServiceCallback() = default;
public:
    virtual void onCommandMessageReceived(const std::string& message) = 0;
};
}