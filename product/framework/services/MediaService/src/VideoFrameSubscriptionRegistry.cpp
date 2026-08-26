#include "VideoFrameSubscriptionRegistry.h"

#include <algorithm>
#include <exception>
#include <utility>

#include <ucf/utilities/UUIDUtils/UUIDUtils.h>

#include "MediaServiceLogger.h"

namespace ucf::service {

namespace {

// Identifies only the subscription currently executing on this thread. This
// lets a callback remove itself without weakening the wait guarantee for
// removals initiated by other threads.
thread_local const void* currentSubscription = nullptr;

} // namespace

std::string VideoFrameSubscriptionRegistry::add(VideoFrameCallback callback)
{
    if (!callback)
    {
        SERVICE_LOG_WARN("cannot add an empty camera frame callback");
        return {};
    }

    auto subscription = std::make_shared<SubscriptionState>();
    subscription->id = ucf::utilities::UUIDUtils::generateUUID();
    subscription->callback = std::move(callback);

    {
        std::scoped_lock lock(mMutex);
        mSubscriptions.push_back(subscription);
    }

    return subscription->id;
}

bool VideoFrameSubscriptionRegistry::remove(const std::string& subscriptionId)
{
    SubscriptionPtr removedSubscription;
    {
        std::scoped_lock lock(mMutex);
        const auto iter = std::find_if(
            mSubscriptions.begin(),
            mSubscriptions.end(),
            [&subscriptionId](const SubscriptionPtr& subscription) {
                return subscription->id == subscriptionId;
            });

        if (iter == mSubscriptions.end())
        {
            return false;
        }

        removedSubscription = *iter;
        mSubscriptions.erase(iter);
    }

    std::unique_lock lock(removedSubscription->mutex);
    removedSubscription->active = false;

    const bool removingCurrentCallback =
        currentSubscription == removedSubscription.get();
    const bool waitsForCallback =
        !removingCurrentCallback && removedSubscription->inFlight > 0;
    if (waitsForCallback)
    {
        SERVICE_LOG_DEBUG("waiting for camera callback to finish, subscriptionId: "
            << subscriptionId << ", inFlight: " << removedSubscription->inFlight);
    }

    if (!removingCurrentCallback)
    {
        removedSubscription->changed.wait(lock, [&removedSubscription] {
            return removedSubscription->inFlight == 0;
        });
    }
    if (waitsForCallback)
    {
        SERVICE_LOG_DEBUG("camera callback finished; subscription removal can continue, subscriptionId: "
            << subscriptionId);
    }

    return true;
}

void VideoFrameSubscriptionRegistry::dispatch(const media::IVideoFramePtr& frame)
{
    if (!frame)
    {
        return;
    }

    const auto subscriptions = snapshot();
    for (const auto& subscription : subscriptions)
    {
        VideoFrameCallback callback;
        {
            std::scoped_lock lock(subscription->mutex);
            if (!subscription->active || !subscription->callback)
            {
                continue;
            }

            callback = subscription->callback;
            ++subscription->inFlight;
        }

        const auto* previousSubscription = currentSubscription;
        currentSubscription = subscription.get();

        try
        {
            callback(frame);
        }
        catch (const std::exception& exception)
        {
            SERVICE_LOG_ERROR("camera frame callback threw, subscriptionId: "
                << subscription->id << ", error: " << exception.what());
        }
        catch (...)
        {
            SERVICE_LOG_ERROR("camera frame callback threw an unknown exception, subscriptionId: "
                << subscription->id);
        }

        currentSubscription = previousSubscription;

        {
            std::scoped_lock lock(subscription->mutex);
            --subscription->inFlight;
            if (subscription->inFlight == 0)
            {
                subscription->changed.notify_all();
            }
        }
    }
}

void VideoFrameSubscriptionRegistry::deactivateAll()
{
    std::vector<SubscriptionPtr> subscriptions;
    {
        std::scoped_lock lock(mMutex);
        subscriptions.swap(mSubscriptions);
    }

    for (const auto& subscription : subscriptions)
    {
        std::scoped_lock lock(subscription->mutex);
        subscription->active = false;
    }

    if (!subscriptions.empty())
    {
        SERVICE_LOG_DEBUG("deactivated camera subscriptions, count: " << subscriptions.size());
    }
}

std::vector<VideoFrameSubscriptionRegistry::SubscriptionPtr>
VideoFrameSubscriptionRegistry::snapshot() const
{
    std::scoped_lock lock(mMutex);
    return mSubscriptions;
}

} // namespace ucf::service
