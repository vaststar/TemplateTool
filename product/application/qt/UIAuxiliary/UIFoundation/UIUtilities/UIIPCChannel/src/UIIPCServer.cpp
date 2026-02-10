#include <UIIPCChannel/UIIPCServer.h>

#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

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
                    if (std::string dataMessage = client->readAll().toStdString(); !dataMessage.empty())
                    {
                        UIIPCChannel_LOG_DEBUG("UIIPCServer receive message:" << dataMessage);
                        if (handler)
                        {
                            handler(dataMessage);
                        }
                        else
                        {
                            UIIPCChannel_LOG_WARN("no handler set");
                        }
                    }
                });
                QObject::connect(client, &QLocalSocket::disconnected, client, &QLocalSocket::deleteLater);
            }
        });
        UIIPCChannel_LOG_INFO("UIIPCServer listen succeed, serverName:" << serverName.toStdString());
        return true;
    }

    void setMessageHandler(MessageHandler h)
    {
        handler = std::move(h);
    }

    void stop()
    {
        UIIPCChannel_LOG_INFO("will stop server");
        if (server.isListening())
        {
            UIIPCChannel_LOG_INFO("close server");
            server.close();
        }
        UIIPCChannel_LOG_INFO("remove server name: " << serverName.toStdString());
        QLocalServer::removeServer(serverName);
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

UIIPCServer::~UIIPCServer()
{
    pImpl->stop();
}

bool UIIPCServer::start()
{
    return pImpl->start();
}

void UIIPCServer::setMessageHandler(MessageHandler handler)
{
    pImpl->setMessageHandler(handler);
}
}
