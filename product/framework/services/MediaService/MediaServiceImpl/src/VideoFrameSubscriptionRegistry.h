#pragma once

#include <cstddef>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <ucf/services/MediaService/IMediaService.h>

namespace ucf::service {

class VideoFrameSubscriptionRegistry
{
public:
    VideoFrameSubscriptionRegistry() = default;
    VideoFrameSubscriptionRegistry(const VideoFrameSubscriptionRegistry&) = delete;
    VideoFrameSubscriptionRegistry(VideoFrameSubscriptionRegistry&&) = delete;
    VideoFrameSubscriptionRegistry& operator=(const VideoFrameSubscriptionRegistry&) = delete;
    VideoFrameSubscriptionRegistry& operator=(VideoFrameSubscriptionRegistry&&) = delete;
    ~VideoFrameSubscriptionRegistry() = default;

    [[nodiscard]] std::string add(VideoFrameCallback callback);

    // External callers wait for an in-flight callback to finish. A callback
    // may remove itself without waiting on itself.
    bool remove(const std::string& subscriptionId);

    // Dispatches against a stable snapshot without holding the registry or
    // per-subscription mutex while application code is running.
    void dispatch(const media::IVideoFramePtr& frame);

    // Prevents callbacks from a previously captured snapshot from starting.
    void deactivateAll();

private:
    struct SubscriptionState
    {
        std::string id;
        VideoFrameCallback callback;

        std::mutex mutex;
        std::condition_variable changed;
        bool active{true};
        std::size_t inFlight{0};
    };

    using SubscriptionPtr = std::shared_ptr<SubscriptionState>;

    [[nodiscard]] std::vector<SubscriptionPtr> snapshot() const;

private:
    mutable std::mutex mMutex;
    std::vector<SubscriptionPtr> mSubscriptions;
};

} // namespace ucf::service
