#include "ipc_channel.h"

#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include <QtNetwork/QLocalServer>

#include "LoggerDefine.h"

namespace UIUtilities{
class UIIPCServer::Impl
{
public:
    explicit Impl(const std::string& name)
        : serverName(QString::fromUtf8(name.c_str()))
    {
    }

    bool start()
    {
        QLocalServer::removeServer(serverName);

        if (!server.listen(serverName))
        {
            UIIPCChannel_LOG_INFO("UIIPCServer listen failed:" << server.errorString().toStdString());
            return false;
        }

        QObject::connect(&server, &QLocalServer::newConnection, &server, [this]() {
            while (auto* client = server.nextPendingConnection())
            {
                QObject::connect(client, &QLocalSocket::readyRead, client, [this, client]() {
                    if (const QByteArray data = client->readAll(); handler)
                    {
                        UIIPCChannel_LOG_DEBUG("UIIPCServer receive message:" << server.errorString().toStdString());
                        handler(data.toStdString());
                    }
                });
                QObject::connect(client, &QLocalSocket::disconnected, client, &QLocalSocket::deleteLater);
            }
        });

        return true;
    }

    void setMessageHandler(MessageHandler h)
    {
        handler = std::move(h);
    }
private:
    QString serverName;
    QLocalServer server;
    MessageHandler handler;
};

UIIPCServer::UIIPCServer(std::string serverName)
    : pImpl(std::make_unique<Impl>(serverName))
{
}

UIIPCServer::~UIIPCServer() = default;

bool UIIPCServer::start()
{
    return pImpl->start();
}

void UIIPCServer::setMessageHandler(MessageHandler handler)
{
    pImpl->setMessageHandler(handler);
}
}
