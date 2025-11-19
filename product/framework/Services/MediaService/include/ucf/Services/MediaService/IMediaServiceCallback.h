#pragma once

#include <vector>

namespace ucf::service{
class IMediaServiceCallback
{
public:
    IMediaServiceCallback() = default;
    IMediaServiceCallback(const IMediaServiceCallback&) = delete;
    IMediaServiceCallback(IMediaServiceCallback&&) = delete;
    IMediaServiceCallback& operator=(const IMediaServiceCallback&) = delete;
    IMediaServiceCallback& operator=(IMediaServiceCallback&&) = delete;
    virtual ~ IMediaServiceCallback() = default;
};
}