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


void CoreViewFactory::loadQmlWindow(const QString& qmlResource, const ControllerCallback& controllerCallback)
{
    QObject::connect(mQmlEngine.get(), &QQmlApplicationEngine::objectCreated, [this,qmlResource, controllerCallback](QObject* object, const QUrl& url) {
        if (object && url.toString() == qmlResource)
        {
            UICore_LOG_DEBUG("object created: " << url.toString().toStdString());
            if (auto controller = object->findChild<CoreController*>())
            {
                UICore_LOG_DEBUG("controller found, name: " << controller->getControllerName().toStdString());
                controllerCallback(controller);
            }
        }
        else
        {
            UICore_LOG_WARN("object created failed: " << qmlResource.toStdString());
        }
    });
    mQmlEngine->load(qmlResource);
}   