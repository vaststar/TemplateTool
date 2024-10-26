#include <UICore/CoreViewFactory.h>
#include <UICore/CoreQmlApplicationEngine.h>
#include <UICore/CoreController.h>

#include <QFileInfo>
#include <iostream>
#include "LoggerDefine.h"
CoreViewFactory::CoreViewFactory(std::unique_ptr<CoreQmlApplicationEngine>&& qmlEngine)
    : mQmlEngine(std::move(qmlEngine))
{
}

CoreViewFactory::~CoreViewFactory() = default;

QString CoreViewFactory::getQRCPrefixPath() const
{
    return QStringLiteral("qrc:/qt/qml/");
}

QPointer<QQuickView> CoreViewFactory::createQmlWindow(const QString& qmlResource, QWindow* parent, QObject* controller)
{
    if (qmlResource.isEmpty())
    {
        return nullptr;
    }

    auto view = new QQuickView(mQmlEngine.get(), parent);

    if (controller)
    {
        view->setInitialProperties({ {QStringLiteral("controller"), QVariant::fromValue(controller)} });
    }

    view->setSource(qmlResource);
    return view;
}


void CoreViewFactory::loadQmlWindow(const QString& qmlResource, const QString& controllerObjectName, const ControllerCallback& controllerCallback)
{
    QString actualQmlResource;
    if (qmlResource.startsWith("qrc:/"))
    {
        actualQmlResource = qmlResource;
    }
    else
    {
        actualQmlResource = getQRCPrefixPath() + qmlResource;
    }
    
    QObject::connect(mQmlEngine.get(), &QQmlApplicationEngine::objectCreated, [actualQmlResource, controllerObjectName, controllerCallback](QObject* object, const QUrl& url) {
        if (object && url.toString() == actualQmlResource)
        {
            UICore_LOG_DEBUG("object created: " << url.toString().toStdString());
            if (auto controller = object->findChild<CoreController*>(controllerObjectName))
            {
                UICore_LOG_DEBUG("controller found, controllerName: " << controller->getControllerName().toStdString() << ", objectName: "<<controller->objectName().toStdString());
                controllerCallback(controller);
            }
        }
        else
        {
            UICore_LOG_WARN("object created failed: " << actualQmlResource.toStdString());
        }
    });
    mQmlEngine->load(actualQmlResource);
}   