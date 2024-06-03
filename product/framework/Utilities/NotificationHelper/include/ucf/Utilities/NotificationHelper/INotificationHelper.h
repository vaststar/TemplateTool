#pragma once

namespace ucf::utilities{
template <typename CallbackObject>
class INotificationHelper
{
public:
    using Callback = CallbackObject;
    using CallbackWeakPtr = std::weak_ptr<CallbackObject>;
    using CallbackPtr = std::shared_ptr<CallbackObject>;
    using CallbackList = std::vector<CallbackWeakPtr>;

    virtual void registerCallback(CallbackPtr callback) = 0;
    virtual void unRegisterCallback(CallbackPtr callback) = 0;
};
}