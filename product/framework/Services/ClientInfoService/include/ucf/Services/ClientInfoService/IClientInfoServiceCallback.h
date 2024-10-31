#pragma once

#include <vector>

namespace ucf::service{

class SERVICE_EXPORT IClientInfoServiceCallback
{
public:
    virtual ~ IClientInfoServiceCallback() = default;
    virtual void onClientInfoChanged() = 0;
};
}