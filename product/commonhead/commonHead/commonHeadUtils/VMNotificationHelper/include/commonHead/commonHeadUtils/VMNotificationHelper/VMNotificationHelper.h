#pragma once

#pragma once

#include <mutex>
#include <memory>
#include <algorithm>
#include <functional>

#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>

namespace commonHead::utilities{
template <typename CallbackObject>
class  VMNotificationHelper: public virtual IVMNotificationHelper<CallbackObject>
{
public:
    using Callback = typename IVMNotificationHelper<CallbackObject>::Callback;
    using CallbackWeakPtr = typename IVMNotificationHelper<CallbackObject>::CallbackWeakPtr;
    using CallbackPtr = typename IVMNotificationHelper<CallbackObject>::CallbackPtr;
    using CallbackList = typename IVMNotificationHelper<CallbackObject>::CallbackList;
    void registerCallback(CallbackPtr callback) override
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

    void unRegisterCallback(CallbackPtr callback) override
    {
        if (!callback)
        {
            return;
        }
        std::scoped_lock loc(mDataMutex);
        mCallbacks.erase(std::remove_if(mCallbacks.begin(), mCallbacks.end(),[callback](CallbackWeakPtr weakCallback){
            if (auto callbackPtr = weakCallback.lock())
            {
                return callback == callbackPtr;
            }
            return false;
        }),mCallbacks.end());
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
}