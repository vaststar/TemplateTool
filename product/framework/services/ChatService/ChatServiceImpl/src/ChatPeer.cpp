#include "ChatPeer.h"

#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/x509.h>
#include <rtc/rtc.hpp>

#include <array>
#include <fstream>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <variant>

#if defined(_WIN32)
#include <Windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace ucf::service::chat {
namespace {

constexpr auto ChannelLabel = "templatetool-chat";
constexpr auto ChannelProtocol = "templatetool-chat-v1";
constexpr std::size_t MaximumFrameBytes = 64 * 1024;
constexpr std::size_t MaximumSendBufferBytes = 256 * 1024;
constexpr std::size_t MaximumIdentityBytes = 32 * 1024;
constexpr std::string_view KeyMarker = "-----BEGIN PRIVATE KEY-----";
constexpr char HexDigits[] = "0123456789ABCDEF";

using Bio = std::unique_ptr<BIO, decltype(&BIO_free)>;
using Certificate = std::unique_ptr<X509, decltype(&X509_free)>;
using Key = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;

std::string hexBytes(const unsigned char* bytes, std::size_t size)
{
    std::string result;
    result.reserve(size * 2);
    for (std::size_t i = 0; i < size; ++i) {
        result.push_back(HexDigits[bytes[i] >> 4]);
        result.push_back(HexDigits[bytes[i] & 15]);
    }
    return result;
}

std::array<unsigned char, 16> randomBytes()
{
    std::array<unsigned char, 16> bytes{};
    if (RAND_bytes(bytes.data(), static_cast<int>(bytes.size())) != 1) {
        throw std::runtime_error("Could not generate device identity");
    }
    return bytes;
}

std::string fingerprintOf(X509* certificate)
{
    std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
    unsigned int size = 0;
    if (X509_digest(certificate, EVP_sha256(), digest.data(), &size) != 1 || size != 32) {
        throw std::runtime_error("Could not read device identity fingerprint");
    }
    return hexBytes(digest.data(), size);
}

std::optional<std::string> fingerprintOf(const rtc::CertificateFingerprint& fingerprint)
{
    if (fingerprint.algorithm != rtc::CertificateFingerprint::Algorithm::Sha256 || !fingerprint.isValid()) {
        return std::nullopt;
    }
    std::string normalized;
    normalized.reserve(64);
    for (char ch : fingerprint.value) {
        if (ch >= 'a' && ch <= 'f') {
            ch = static_cast<char>(ch - 'a' + 'A');
        }
        if (ch != ':') {
            normalized.push_back(ch);
        }
    }
    if (normalized.size() != 64) {
        return std::nullopt;
    }
    return normalized;
}

std::string readIdentity(const std::filesystem::path& path)
{
    std::error_code error;
    const auto status = std::filesystem::symlink_status(path, error);
    if (error || !std::filesystem::is_regular_file(status)) {
        throw std::runtime_error("Could not read device identity");
    }
#if !defined(_WIN32)
    using Permissions = std::filesystem::perms;
    if ((status.permissions() & (Permissions::group_all | Permissions::others_all)) != Permissions::none) {
        throw std::runtime_error("Device identity file permissions are too broad");
    }
#endif
    const auto size = std::filesystem::file_size(path, error);
    if (error || size == 0 || size > MaximumIdentityBytes) {
        throw std::runtime_error("Device identity file has an invalid size");
    }
    std::ifstream file(path, std::ios::binary);
    std::string result(static_cast<std::size_t>(size), '\0');
    if (!file.read(result.data(), static_cast<std::streamsize>(size))) {
        throw std::runtime_error("Could not read device identity");
    }
    return result;
}

std::string validateIdentity(const std::string& pem)
{
    const auto keyStart = pem.find(KeyMarker);
    if (keyStart == std::string::npos) {
        throw std::runtime_error("Device identity file is invalid");
    }
    Bio certificateInput(BIO_new_mem_buf(pem.data(), static_cast<int>(keyStart)), BIO_free);
    Bio keyInput(BIO_new_mem_buf(pem.data() + keyStart, static_cast<int>(pem.size() - keyStart)), BIO_free);
    if (!certificateInput || !keyInput) {
        throw std::runtime_error("Could not read device identity");
    }
    Certificate certificate(PEM_read_bio_X509(certificateInput.get(), nullptr, nullptr, nullptr), X509_free);
    auto rejectPasswordPrompt = [](char*, int, int, void*) { return 0; };
    Key key(PEM_read_bio_PrivateKey(keyInput.get(), nullptr, rejectPasswordPrompt, nullptr), EVP_PKEY_free);
    if (!certificate || !key || X509_check_private_key(certificate.get(), key.get()) != 1) {
        throw std::runtime_error("Device identity file is invalid");
    }
    return fingerprintOf(certificate.get());
}

std::string generateIdentity()
{
    std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> context(
        EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr), EVP_PKEY_CTX_free);
    EVP_PKEY* generatedKey = nullptr;
    if (!context || EVP_PKEY_keygen_init(context.get()) != 1 ||
        EVP_PKEY_CTX_set_ec_paramgen_curve_nid(context.get(), NID_X9_62_prime256v1) != 1 ||
        EVP_PKEY_keygen(context.get(), &generatedKey) != 1) {
        throw std::runtime_error("Could not generate device key");
    }
    Key key(generatedKey, EVP_PKEY_free);
    Certificate certificate(X509_new(), X509_free);
    auto serialBytes = randomBytes();
    serialBytes[0] = static_cast<unsigned char>((serialBytes[0] & 0x7f) | 1);
    std::unique_ptr<BIGNUM, decltype(&BN_free)> serial(
        BN_bin2bn(serialBytes.data(), static_cast<int>(serialBytes.size()), nullptr), BN_free);
    if (!certificate || !serial || X509_set_version(certificate.get(), 2) != 1 ||
        !BN_to_ASN1_INTEGER(serial.get(), X509_get_serialNumber(certificate.get())) ||
        !X509_gmtime_adj(X509_getm_notBefore(certificate.get()), -86400) ||
        !X509_gmtime_adj(X509_getm_notAfter(certificate.get()), 10L * 365 * 86400) ||
        X509_set_pubkey(certificate.get(), key.get()) != 1) {
        throw std::runtime_error("Could not generate device certificate");
    }
    constexpr unsigned char name[] = "TemplateTool Chat";
    auto* subject = X509_get_subject_name(certificate.get());
    if (!subject || X509_NAME_add_entry_by_txt(subject, "CN", MBSTRING_ASC, name, -1, -1, 0) != 1 ||
        X509_set_issuer_name(certificate.get(), subject) != 1 ||
        X509_sign(certificate.get(), key.get(), EVP_sha256()) <= 0) {
        throw std::runtime_error("Could not sign device certificate");
    }
    Bio output(BIO_new(BIO_s_mem()), BIO_free);
    if (!output || PEM_write_bio_X509(output.get(), certificate.get()) != 1 ||
        PEM_write_bio_PrivateKey(output.get(), key.get(), nullptr, nullptr, 0, nullptr, nullptr) != 1) {
        throw std::runtime_error("Could not encode device identity");
    }
    char* data = nullptr;
    const auto size = BIO_get_mem_data(output.get(), &data);
    if (!data || size <= 0) {
        throw std::runtime_error("Could not encode device identity");
    }
    return std::string(data, static_cast<std::size_t>(size));
}

void publishIdentity(const std::filesystem::path& path, const std::string& pem)
{
    auto random = randomBytes();
    const auto temporary = path.parent_path() / ("identity-" + hexBytes(random.data(), random.size()) + ".tmp");
#if defined(_WIN32)
    const auto file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Could not create device identity file");
    }
    DWORD size = 0;
    const bool written = WriteFile(file, pem.data(), static_cast<DWORD>(pem.size()), &size, nullptr) &&
                         size == pem.size() && FlushFileBuffers(file);
    const bool closed = CloseHandle(file);
    const bool moved = written && closed && MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_WRITE_THROUGH);
    const auto error = GetLastError();
    if (moved) {
        return;
    }
    DeleteFileW(temporary.c_str());
    if (!written || !closed || (error != ERROR_FILE_EXISTS && error != ERROR_ALREADY_EXISTS)) {
        throw std::runtime_error("Could not save device identity file");
    }
#else
    const auto file = open(temporary.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (file < 0) {
        throw std::runtime_error("Could not create device identity file");
    }
    const bool written = write(file, pem.data(), pem.size()) == static_cast<ssize_t>(pem.size()) && fsync(file) == 0;
    const bool closed = close(file) == 0;
    const bool linked = written && closed && link(temporary.c_str(), path.c_str()) == 0;
    const auto error = errno;
    unlink(temporary.c_str());
    if (!linked && (!written || !closed || error != EEXIST)) {
        throw std::runtime_error("Could not save device identity file");
    }
#endif
}

rtc::Configuration makeConfiguration(const ChatIdentity& identity)
{
    auto pem = readIdentity(identity.certificatePath);
    if (identity.certificatePath != identity.keyPath || validateIdentity(pem) != identity.fingerprint) {
        throw std::runtime_error("Device identity has changed");
    }
    const auto keyStart = pem.find(KeyMarker);
    rtc::Configuration config;
    // Native paths are opened above. Passing inline PEM also supports non-ASCII Windows profiles.
    config.certificatePemFile = pem.substr(0, keyStart);
    config.keyPemFile = pem.substr(keyStart);
    config.disableAutoNegotiation = true;
    config.disableFingerprintVerification = false;
    config.maxMessageSize = MaximumFrameBytes;
    return config;
}

} // namespace

ChatIdentity loadOrCreateChatIdentity(const std::filesystem::path& directory)
{
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error) {
        throw std::runtime_error("Could not prepare device identity directory");
    }
    const auto path = directory / "identity.pem";
    const auto status = std::filesystem::symlink_status(path, error);
    if (status.type() == std::filesystem::file_type::not_found &&
        (!error || error == std::errc::no_such_file_or_directory)) {
        // Publish the certificate and key as one file so another process sees a complete pair.
        publishIdentity(path, generateIdentity());
    }
    return {path, path, validateIdentity(readIdentity(path))};
}

class ChatPeer::Impl final : public std::enable_shared_from_this<Impl> {
public:
    Impl(const ChatIdentity& identity, Events events)
        : mConnection(std::make_shared<rtc::PeerConnection>(makeConfiguration(identity))), mEvents(std::move(events))
    {
    }

    void bind()
    {
        auto weak = weak_from_this();
        mConnection->onGatheringStateChange([weak](rtc::PeerConnection::GatheringState state) {
            if (auto self = weak.lock(); self && state == rtc::PeerConnection::GatheringState::Complete) {
                self->localDescription();
            }
        });
        mConnection->onDataChannel([weak](std::shared_ptr<rtc::DataChannel> channel) {
            if (auto self = weak.lock()) {
                self->attachChannel(std::move(channel));
            }
        });
        mConnection->onStateChange([weak](rtc::PeerConnection::State state) {
            if (auto self = weak.lock()) {
                using State = rtc::PeerConnection::State;
                if (state == State::Failed) {
                    self->closed("Peer connection failed");
                } else if (state == State::Disconnected) {
                    self->closed("Peer disconnected");
                } else if (state == State::Closed) {
                    self->closed({});
                }
            }
        });
    }

    void startOffer()
    {
        rtc::DataChannelInit options;
        options.protocol = ChannelProtocol;
        attachChannel(mConnection->createDataChannel(ChannelLabel, std::move(options)));
        mConnection->setLocalDescription(rtc::Description::Type::Offer);
    }

    void setRemoteDescription(const std::string& sdp, bool offer)
    {
        using Type = rtc::Description::Type;
        auto description = rtc::Description(sdp, offer ? Type::Offer : Type::Answer);
        const auto fingerprint = description.fingerprint();
        const auto normalized = fingerprint ? fingerprintOf(*fingerprint) : std::nullopt;
        if (!description.hasApplication() || !normalized) {
            throw std::runtime_error("Invalid peer description");
        }
        {
            std::lock_guard lock(mMutex);
            if (mClosed || mExpectedFingerprint) {
                throw std::runtime_error("Peer description already set");
            }
            mExpectedFingerprint = normalized;
        }
        mConnection->setRemoteDescription(std::move(description));
        if (offer) {
            mConnection->setLocalDescription(Type::Answer);
        }
    }

    bool send(const std::string& bytes)
    {
        std::shared_ptr<rtc::DataChannel> channel;
        {
            std::lock_guard lock(mMutex);
            if (mClosed || !mOpened) {
                return false;
            }
            channel = mChannel;
        }
        if (!channel || bytes.empty() || bytes.size() > MaximumFrameBytes ||
            bytes.size() > channel->maxMessageSize() || !channel->isOpen() ||
            channel->bufferedAmount() > MaximumSendBufferBytes - bytes.size()) {
            return false;
        }
        // rtc::send returns false when buffered. The service tracks delivery through its own ACKs.
        channel->send(bytes);
        return true;
    }

    void shutdown() noexcept
    {
        std::shared_ptr<rtc::DataChannel> channel;
        {
            std::lock_guard lock(mMutex);
            mClosed = true;
            channel = std::move(mChannel);
        }
        try {
            mConnection->resetCallbacks();
            if (channel) {
                channel->resetCallbacks();
            }
            mConnection->close();
        } catch (...) {
            // Teardown should continue when the underlying transport is already closing.
        }
    }

private:
    void attachChannel(std::shared_ptr<rtc::DataChannel> channel)
    {
        if (channel->label() != ChannelLabel || channel->protocol() != ChannelProtocol) {
            channel->close();
            return;
        }
        {
            std::lock_guard lock(mMutex);
            if (mClosed || mChannel) {
                return;
            }
            mChannel = channel;
        }
        auto weak = weak_from_this();
        channel->onOpen([weak] {
            if (auto self = weak.lock()) {
                self->opened();
            }
        });
        channel->onMessage([weak](rtc::message_variant message) {
            if (auto self = weak.lock()) {
                self->received(std::move(message));
            }
        });
        channel->onClosed([weak] {
            if (auto self = weak.lock()) {
                self->closed({});
            }
        });
        channel->onError([weak](std::string) {
            if (auto self = weak.lock()) {
                self->closed("Peer channel failed");
            }
        });
    }

    void localDescription()
    {
        auto description = mConnection->localDescription();
        if (!description) {
            closed("Could not prepare peer description");
            return;
        }
        std::lock_guard lock(mMutex);
        if (!mClosed && mEvents.onLocalDescription) {
            mEvents.onLocalDescription(static_cast<std::string>(*description));
        }
    }

    void opened()
    {
        const auto verified = fingerprintOf(mConnection->remoteFingerprint());
        std::lock_guard lock(mMutex);
        if (mClosed || mOpened) {
            return;
        }
        if (!verified || verified != mExpectedFingerprint) {
            reportClosed("Peer certificate verification failed");
            return;
        }
        mOpened = true;
        if (mEvents.onOpen) {
            mEvents.onOpen(*verified);
        }
    }

    void received(rtc::message_variant message)
    {
        auto* text = std::get_if<std::string>(&message);
        if (!text || text->empty() || text->size() > MaximumFrameBytes) {
            closed("Invalid peer message");
            return;
        }
        std::lock_guard lock(mMutex);
        if (!mClosed && mOpened && mEvents.onMessage) {
            mEvents.onMessage(std::move(*text));
        }
    }

    void closed(std::string reason)
    {
        std::lock_guard lock(mMutex);
        reportClosed(std::move(reason));
    }

    void reportClosed(std::string reason)
    {
        if (mClosed) {
            return;
        }
        mClosed = true;
        if (mEvents.onClosed) {
            mEvents.onClosed(std::move(reason));
        }
    }

    std::shared_ptr<rtc::PeerConnection> mConnection;
    Events mEvents;
    std::mutex mMutex;
    std::shared_ptr<rtc::DataChannel> mChannel;
    std::optional<std::string> mExpectedFingerprint;
    bool mOpened = false;
    bool mClosed = false;
};

ChatPeer::ChatPeer(const ChatIdentity& identity, Events events)
{
    try {
        mImpl = std::make_shared<Impl>(identity, std::move(events));
        mImpl->bind();
    } catch (const std::exception&) {
        throw std::runtime_error("Could not start peer connection");
    }
}

ChatPeer::~ChatPeer()
{
    mImpl->shutdown();
}

void ChatPeer::startOffer()
{
    try {
        mImpl->startOffer();
    } catch (const std::exception&) {
        throw std::runtime_error("Could not create peer offer");
    }
}

void ChatPeer::setRemoteDescription(const std::string& sdp, bool offer)
{
    try {
        mImpl->setRemoteDescription(sdp, offer);
    } catch (const std::exception&) {
        throw std::runtime_error("Could not apply peer description");
    }
}

bool ChatPeer::send(const std::string& bytes)
{
    try {
        return mImpl->send(bytes);
    } catch (const std::exception&) {
        return false;
    }
}

} // namespace ucf::service::chat
