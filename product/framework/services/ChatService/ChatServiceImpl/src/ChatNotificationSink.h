#pragma once

namespace ucf::service::chat {

class IChatNotificationSink
{
public:
    virtual ~IChatNotificationSink() = default;
    virtual void onChatChanged() = 0;
};

} // namespace ucf::service::chat
