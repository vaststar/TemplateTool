#pragma once

#include <ucf/Services/MediaService/MediaTypes.h>
#include <vector>

namespace ucf::service::media {

class VideoFrame : public IVideoFrame
{
public:
    VideoFrame() = default;
    
    VideoFrame(std::vector<uint8_t> data, int width, int height, int bytesPerLine,
               PixelFormat format = PixelFormat::RGB888);

    [[nodiscard]] const uint8_t* getData() const override;
    [[nodiscard]] size_t getDataSize() const override;
    [[nodiscard]] int getWidth() const override;
    [[nodiscard]] int getHeight() const override;
    [[nodiscard]] int getBytesPerLine() const override;
    [[nodiscard]] PixelFormat getFormat() const override;
    [[nodiscard]] int64_t getTimestampMs() const override;
    [[nodiscard]] uint32_t getFrameIndex() const override;
    [[nodiscard]] bool isValid() const override;

    // 内部使用的 setter
    void setTimestamp(int64_t ts);
    void setFrameIndex(uint32_t idx);

private:
    std::vector<uint8_t> mData{};
    int mWidth{0};
    int mHeight{0};
    int mBytesPerLine{0};
    PixelFormat mFormat{PixelFormat::RGB888};
    int64_t mTimestampMs{0};
    uint32_t mFrameIndex{0};
};

}  // namespace ucf::service::media
