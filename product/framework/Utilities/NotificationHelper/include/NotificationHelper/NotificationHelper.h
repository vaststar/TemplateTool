#pragma once

#include <mutex>
#include <memory>
#include <algorithm>
#include <functional>

#include "NotificationHelper/NotificationHelperExport.h"

template <typename CallbackObject>
class Notification_LIB_API NotificationHelper
{
public:
    using Callback = CallbackObject;
    using CallbackWeakPtr = std::weak_ptr<CallbackObject>;
    using CallbackPtr = std::shared_ptr<CallbackObject>;
    using CallbackList = std::vector<CallbackWeakPtr>;

    virtual ~NotificationHelper() = default;

    void registerCallback(CallbackPtr callback)
    {
        if (!callback)
        {
            return;
        }
        std::scoped_lock loc(mDataMutex);
        if (std::find_if(mCallbacks.cbegin(), mCallbacks.cend(), [callback](CallbackWeakPtr weakCallback){
            if (auto callbackPtr = weakCallback.lock())
            {
                return callback == callbackPtr;
            }
            return false;
        }) == mCallbacks.cend())
        {
            mCallbacks.push_back(callback);
        }
    }
protected:
    template <typename Func, typename ...Args>
    void fireNotification(Func func, Args&&... args)
    {
        CallbackList callbacks = removeExpiredCallbacks();
        for (const auto& callback: callbacks)
        {
            if (auto lockedCallback = callback.lock())
            {
                std::invoke(func, lockedCallback, std::forward<Args>(args)...);
            }
        }
    }
private:
    CallbackList removeExpiredCallbacks()
    {
        CallbackList livedCallbacks;
        {
            std::scoped_lock loc(mDataMutex);
            if (mCallbacks.empty())
            {
                return {};
            }

            for(auto callbackIter = mCallbacks.begin(); callbackIter != mCallbacks.end();)
            {
                if ((*callbackIter).expired())
                {
                    callbackIter = mCallbacks.erase(callbackIter);
                }
                else
                {
                    livedCallbacks.push_back(*callbackIter);
                    ++callbackIter;
                }
            }
        }
        return livedCallbacks;
    }
private:
    std::mutex mDataMutex;
    CallbackList mCallbacks;
};