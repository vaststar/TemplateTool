#pragma once

#include <memory>
#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>
#include <ucf/Utilities/TcpChannelUtils/TcpChannelConfig.h>
#include <ucf/Utilities/NotificationHelper/INotificationHelper.h>

namespace ucf::utilities {

class ITcpChannelCallback;

/// Single-client TCP server channel for IPC communication.
///
/// Listens on a local port, accepts one client connection,
/// and provides bidirectional byte-stream communication.
/// This class does NOT handle message framing â€” the caller
/// is responsible for delimiting messages (e.g. newline-delimited JSON).
///
/// Typical usage:
/// @code
///   auto channel = ITcpChannel::create();
///   channel->registerCallback(myCallback);
///
///   TcpChannelConfig config;
///   config.listenPort = 9876;
///   channel->startListening(config);
///
///   // ... client connects, callbacks fire on I/O thread ...
///   channel->send("hello\n");
///
///   channel->stop();
/// @endcode
class Utilities_EXPORT ITcpChannel
    : public virtual INotificationHelper<ITcpChannelCallback>
{
public:
    ~ITcpChannel() override = default;

    /// Start listening for client connections.
    /// @param config  Server configuration (address, port)
    /// @return true if listening started successfully
    virtual bool startListening(const TcpChannelConfig& config) = 0;

    /// Stop listening and disconnect any connected client.
    virtual void stop() = 0;

    /// Send data to the connected client.
    /// @param data  Raw bytes to send
    /// @return true if sent successfully, false if not connected or error
    virtual bool send(const std::string& data) = 0;

    /// Whether a client is currently connected.
    virtual bool isConnected() const = 0;

    /// Whether the server is currently listening.
    virtual bool isListening() const = 0;

    /// The actual port being listened on (useful when config port was 0).
    virtual int listeningPort() const = 0;

    /// Create a TcpChannel instance.
    static std::unique_ptr<ITcpChannel> create();
};

} // namespace ucf::utilities
