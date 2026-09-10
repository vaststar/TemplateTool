#include <ucf/utilities/NetworkTransportUtils/NetworkTransportUtils.h>

#if defined(ASIO_STANDALONE) || defined(ASIO_NO_DEPRECATED)
#  error "NetworkTransportUtils leaked private Asio compile definitions to a consumer"
#endif

int main()
{
    using namespace ucf::utilities::network;

    NetworkRuntime runtime;
    {
        TcpClient client{runtime};
        TcpServer server{runtime};
        UdpSocket socket{runtime};

        if (client.state() != TcpClientState::Idle || server.state() != TcpServerState::Idle || socket.state() != UdpSocketState::Idle)
        {
            return 1;
        }
    }

    runtime.shutdown();
    return 0;
}
