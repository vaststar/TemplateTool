#include "ChatProtocol.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include <nlohmann/json.hpp>

#include "ChatUtils.h"

namespace ucf::service::chat {
namespace {

using Json = nlohmann::json;
constexpr std::string_view invitationPrefix = "TTCHAT1:";
constexpr std::size_t maxInvitationBytes = 64 * 1024;
constexpr std::size_t maxSdpBytes = 48 * 1024;
constexpr std::size_t maxFrameBytes = 6 * maxTextBytes + 256;

bool hexValue(std::string_view value, std::size_t size)
{
    return value.size() == size && std::ranges::all_of(value, [](char ch) {
        return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F');
    });
}

Json readObject(std::string_view bytes)
{
    // The wire envelopes are flat. Abort parsing nested attacker-controlled values
    // before the JSON parser allocates or recursively destroys a deep tree.
    auto flat = [](int depth, Json::parse_event_t, Json&) {
        if (depth > 1) throw std::runtime_error("Unexpected nested chat data.");
        return true;
    };
    auto result = Json::parse(bytes, flat, false);
    if (!result.is_object() || result.value("v", 0) != 1)
    {
        throw std::runtime_error("Unsupported or invalid chat data.");
    }
    return result;
}

} // namespace

std::string encodeInvitation(const Invitation& invitation)
{
    Json data{{"v", 1}, {"type", invitation.answer ? "answer" : "offer"}, {"id", invitation.id},
              {"until", invitation.expiresAtMs}, {"fingerprint", invitation.fingerprint}, {"sdp", invitation.sdp}};
    return std::string(invitationPrefix) + data.dump();
}

Invitation decodeInvitation(std::string_view text, bool expectAnswer)
{
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first != std::string_view::npos) text.remove_prefix(first);
    if (text.size() > maxInvitationBytes || !text.starts_with(invitationPrefix))
    {
        throw std::runtime_error("Paste the complete TemplateTool chat invitation or answer.");
    }

    Invitation invitation;
    std::string kind;
    try
    {
        const auto data = readObject(text.substr(invitationPrefix.size()));
        kind = data.at("type").get<std::string>();
        invitation.id = data.at("id").get<std::string>();
        invitation.expiresAtMs = data.at("until").get<std::int64_t>();
        invitation.fingerprint = data.at("fingerprint").get<std::string>();
        invitation.sdp = data.at("sdp").get<std::string>();
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("The chat invitation or answer is incomplete or unsupported.");
    }

    if (kind != (expectAnswer ? "answer" : "offer"))
    {
        throw std::runtime_error(expectAnswer ? "Paste the other device's answer, not its invitation." :
                                               "Paste an invitation to start a new connection.");
    }
    invitation.answer = expectAnswer;
    const auto time = nowMs();
    if (invitation.expiresAtMs <= time)
    {
        throw std::runtime_error("The invitation has expired. Create a new invitation.");
    }
    if (invitation.expiresAtMs > time + 2 * inviteLifetimeMs || !hexValue(invitation.id, 32) ||
        !hexValue(invitation.fingerprint, 64) || invitation.sdp.size() > maxSdpBytes ||
        !invitation.sdp.starts_with("v=0") || invitation.sdp.find('\0') != std::string::npos ||
        invitation.sdp.find("\nm=application ") == std::string::npos ||
        invitation.sdp.find("\na=candidate:") == std::string::npos)
    {
        throw std::runtime_error("The invitation is invalid or contains no reachable address to try.");
    }
    return invitation;
}

std::string encodeFrame(const Frame& frame)
{
    if (frame.type == FrameType::Ready) return Json{{"v", 1}, {"type", "ready"}}.dump();
    Json data{{"v", 1}, {"type", frame.type == FrameType::Text ? "text" : "ack"}, {"id", frame.id}};
    if (frame.type == FrameType::Text)
    {
        data["text"] = frame.text;
        if (!frame.replyToMessageId.empty()) data["replyTo"] = frame.replyToMessageId;
    }
    return data.dump(-1, ' ', false, Json::error_handler_t::strict);
}

Frame decodeFrame(std::string_view bytes)
{
    if (bytes.size() > maxFrameBytes) throw std::runtime_error("Chat message is too large.");
    const auto data = readObject(bytes);
    const auto type = data.at("type").get<std::string>();
    if (type == "ready") return {FrameType::Ready, {}, {}};
    auto id = data.at("id").get<std::string>();
    if (!hexValue(id, 32)) throw std::runtime_error("Invalid message identifier.");
    if (type == "ack") return {FrameType::Receipt, std::move(id), {}};
    if (type != "text") throw std::runtime_error("Unsupported chat message type.");
    auto text = data.at("text").get<std::string>();
    if (!validText(text, maxTextBytes)) throw std::runtime_error("Invalid chat text.");
    auto replyToMessageId = data.value("replyTo", std::string{});
    if (!replyToMessageId.empty() && (!hexValue(replyToMessageId, 32) || replyToMessageId == id))
        throw std::runtime_error("Invalid reply message identifier.");
    return {FrameType::Text, std::move(id), std::move(text), std::move(replyToMessageId)};
}

} // namespace ucf::service::chat
