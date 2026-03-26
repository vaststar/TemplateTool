#pragma once

#include <string>
#include <ucf/Services/MediaService/MediaTypes.h>

namespace ucf::service {
class IMediaServiceCallback
{
public:
    IMediaServiceCallback() = default;
    IMediaServiceCallback(const IMediaServiceCallback&) = delete;
    IMediaServiceCallback(IMediaServiceCallback&&) = delete;
    IMediaServiceCallback& operator=(const IMediaServiceCallback&) = delete;
    IMediaServiceCallback& operator=(IMediaServiceCallback&&) = delete;
    virtual ~IMediaServiceCallback() = default;
    
    // 视频帧回调
    virtual void onFrameReceived(const std::string& cameraId, const media::IVideoFramePtr& frame) {}
};
}