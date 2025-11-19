#pragma once

namespace ucf::utilities{
template <typename CallbackObject>
class INotificationHelper
{
public:
    INotificationHelper() = default;
    INotificationHelper(const INotificationHelper&) = delete;
    INotificationHelper(INotificationHelper&&) = delete;
    INotificationHelper& operator=(const INotificationHelper&) = delete;
    INotificationHelper& operator=(INotificationHelper&&) = delete;
    virtual ~INotificationHelper() = default;
public:
    using Callback = CallbackObject;
    using CallbackWeakPtr = std::weak_ptr<CallbackObject>;
    using CallbackPtr = std::shared_ptr<CallbackObject>;
    using CallbackList = std::vector<CallbackWeakPtr>;

    virtual void registerCallback(CallbackPtr callback) = 0;
    virtual void unRegisterCallback(CallbackPtr callback) = 0;
};
}