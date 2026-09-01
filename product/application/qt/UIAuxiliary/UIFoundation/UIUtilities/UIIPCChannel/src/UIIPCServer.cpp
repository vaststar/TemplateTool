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
            UIIPCChannel_LOG_ERROR(
                "UIIPCServer listen failed, serverName: "
                << serverName.toStdString()
                << ", error: "
                << server.errorString().toStdString());
            return false;
        }

        QObject::connect(&server, &QLocalServer::newConnection, &server, [this]() {
            while (auto* client = server.nextPendingConnection())
            {
                QObject::connect(client, &QLocalSocket::readyRead, client, [this, client]() {
                    if (std::string dataMessage = client->readAll().toStdString(); !dataMessage.empty())
                    {
                        UIIPCChannel_LOG_DEBUG(
                            "UIIPCServer message received, message: " << dataMessage);
                        if (handler)
                        {
                            handler(dataMessage);
                        }
                        else
                        {
                            UIIPCChannel_LOG_WARN(
                                "UIIPCServer message handling skipped: handler is not configured");
                        }
                    }
                });
                QObject::connect(client, &QLocalSocket::disconnected, client, &QLocalSocket::deleteLater);
            }
        });
        UIIPCChannel_LOG_INFO(
            "UIIPCServer listen succeeded, serverName: "
            << serverName.toStdString());
        return true;
    }

    void setMessageHandler(MessageHandler h)
    {
        handler = std::move(h);
    }

    void stop()
    {
        UIIPCChannel_LOG_INFO(
            "UIIPCServer shutdown started, serverName: "
            << serverName.toStdString());

        if (server.isListening())
        {
            UIIPCChannel_LOG_INFO(
                "UIIPCServer listener close started, serverName: "
                << serverName.toStdString());
            server.close();
            UIIPCChannel_LOG_INFO(
                "UIIPCServer listener close finished, serverName: "
                << serverName.toStdString());
        }

        UIIPCChannel_LOG_INFO(
            "UIIPCServer registered-name removal started, serverName: "
            << serverName.toStdString());
        QLocalServer::removeServer(serverName);
        UIIPCChannel_LOG_INFO(
            "UIIPCServer registered-name removal finished, serverName: "
            << serverName.toStdString());

        UIIPCChannel_LOG_INFO(
            "UIIPCServer shutdown finished, serverName: "
            << serverName.toStdString());
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
