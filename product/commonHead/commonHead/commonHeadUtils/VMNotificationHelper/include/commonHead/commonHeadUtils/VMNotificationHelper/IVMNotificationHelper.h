#pragma once

namespace commonHead::utilities{
template <typename CallbackObject>
class IVMNotificationHelper
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