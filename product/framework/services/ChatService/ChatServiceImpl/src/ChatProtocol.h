#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace ucf::service::chat {

inline constexpr std::int64_t inviteLifetimeMs = 600'000;

struct Invitation
{
    bool answer = false;
    std::string id;
    std::int64_t expiresAtMs = 0;
    std::string fingerprint;
    std::string sdp;
};

enum class FrameType { Ready, Text, Receipt };

struct Frame
{
    FrameType type;
    std::string id;
    std::string text;
    std::string replyToMessageId{};
};

std::string encodeInvitation(const Invitation& invitation);
Invitation decodeInvitation(std::string_view text, bool expectAnswer);
std::string encodeFrame(const Frame& frame);
Frame decodeFrame(std::string_view bytes);

} // namespace ucf::service::chat
