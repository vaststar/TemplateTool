#pragma once

namespace ucf::service {

class IChatServiceCallback
{
public:
    virtual ~IChatServiceCallback() = default;
    virtual void onChatChanged() = 0;
};

} // namespace ucf::service
