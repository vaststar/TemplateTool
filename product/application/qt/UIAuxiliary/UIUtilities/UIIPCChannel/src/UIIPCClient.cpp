#include <UIIPCChannel/UIIPCClient.h>

#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtNetwork/QLocalSocket>

#include "LoggerDefine.h"

namespace UIUtilities{
class UIIPCClient::Impl
{
public:
    Impl(const std::string& name, int timeout)
        : serverName(QString::fromUtf8(name.c_str()))
        , timeoutMs(timeout)
    {

    }
    ~Impl()
    {
        close();
    }

    bool ensureConnected()
    {
        if (socket.state() == QLocalSocket::ConnectedState)
        {
            return true;
        }

        if (socket.state() == QLocalSocket::ConnectingState)
        {
            if (socket.waitForConnected(timeoutMs))
            {
                return true;
            }
        }

        if (socket.state() != QLocalSocket::UnconnectedState)
        {
            socket.abort();
        }

        socket.connectToServer(serverName);
        if (!socket.waitForConnected(timeoutMs))
        {
            UIIPCChannel_LOG_INFO("UIIPCClient connect failed:" << socket.errorString().toStdString());
            return false;
        }
        return true;
    }

    bool sendOne(const std::string& msg)
    {
        if (!ensureConnected())
        {
            return false;
        }

        socket.write(QByteArray::fromStdString(msg));

        if (!socket.waitForBytesWritten(timeoutMs))
        {
            UIIPCChannel_LOG_INFO("UIIPCClient write timeout");
            socket.abort();
            return false;
        }
        return true;
    }

    bool isConnected() const
    {
        return socket.state() == QLocalSocket::ConnectedState;
    }

    void close()
    {
        if (socket.state() == QLocalSocket::ConnectedState)
        {
            socket.disconnectFromServer();
            socket.waitForDisconnected(timeoutMs);
        }
        else
        {
            socket.abort();
        }
    }

private:
    QString serverName;
    int timeoutMs = 1000;
    QLocalSocket socket;
};
UIIPCClient::UIIPCClient(std::string serverName, int timeoutMs)
    : pImpl(std::make_unique<Impl>(serverName, timeoutMs))
{
}

UIIPCClient::~UIIPCClient() = default;

bool UIIPCClient::connect()
{
    return pImpl->ensureConnected();
}

bool UIIPCClient::isConnected() const
{
    return pImpl->isConnected();
}

bool UIIPCClient::send(const std::string& msg)
{
    return pImpl->sendOne(msg);
}

void UIIPCClient::close()
{
    pImpl->close();
}
}
