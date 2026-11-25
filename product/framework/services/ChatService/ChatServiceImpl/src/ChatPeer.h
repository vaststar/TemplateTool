#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <string>

namespace ucf::service::chat {

struct ChatIdentity {
    std::filesystem::path certificatePath;
    std::filesystem::path keyPath;
    std::string fingerprint;
};

ChatIdentity loadOrCreateChatIdentity(const std::filesystem::path& directory);

class ChatPeer final {
public:
    struct Events {
        // RTC worker callbacks must enqueue work and return without reentering ChatPeer.
        std::function<void(std::string)> onLocalDescription;
        std::function<void(std::string)> onOpen;
        std::function<void(std::string)> onMessage;
        std::function<void(std::string)> onClosed;
    };

    ChatPeer(const ChatIdentity& identity, Events events);
    ~ChatPeer();

    ChatPeer(const ChatPeer&) = delete;
    ChatPeer& operator=(const ChatPeer&) = delete;

    void startOffer();
    void setRemoteDescription(const std::string& sdp, bool offer);
    // True when accepted into the transport, including when queued for sending.
    bool send(const std::string& bytes);

private:
    class Impl;
    std::shared_ptr<Impl> mImpl;
};

} // namespace ucf::service::chat
