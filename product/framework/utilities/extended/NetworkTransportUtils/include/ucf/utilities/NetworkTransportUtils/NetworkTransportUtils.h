#pragma once

// Convenience entry point for applications that use more than one transport.
// Individual headers remain available when minimizing includes is preferred.
#include <ucf/utilities/NetworkTransportUtils/core/ByteBuffer.h>
#include <ucf/utilities/NetworkTransportUtils/core/Endpoint.h>
#include <ucf/utilities/NetworkTransportUtils/core/NetworkError.h>
#include <ucf/utilities/NetworkTransportUtils/core/NetworkResult.h>
#include <ucf/utilities/NetworkTransportUtils/core/TransportCloseInfo.h>
#include <ucf/utilities/NetworkTransportUtils/runtime/NetworkRuntime.h>
#include <ucf/utilities/NetworkTransportUtils/tcp/TcpClient.h>
#include <ucf/utilities/NetworkTransportUtils/tcp/TcpServer.h>
#include <ucf/utilities/NetworkTransportUtils/udp/UdpSocket.h>
