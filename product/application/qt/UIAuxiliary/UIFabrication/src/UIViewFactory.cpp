#include <UIFabrication/UIViewFactory.h>
#include <UICore/CoreQmlEngine.h>
#include <UICore/CoreController.h>

#include <QFileInfo>
#include <QApplication>

#include "LoggerDefine.h"


namespace UIFabrication{
UIViewFactory::UIViewFactory(UICore::CoreQmlEngine* qmlEngine)
    : mQmlEngine(qmlEngine)
{
}

UIViewFactory::~UIViewFactory() = default;

QString UIViewFactory::getQRCPrefixPath() const
{
    return QStringLiteral("qrc:/qt/qml/");
}

QString UIViewFactory::generateQmlResourcePath(const QString& qmlResource) const
{
    if (qmlResource.isEmpty())
    {
        return {};
    }

    if (qmlResource.startsWith("qrc:/"))
    {
        return qmlResource;
    }
    else
    {
        return getQRCPrefixPath() + qmlResource;
    }

}

QPointer<QQuickView> UIViewFactory::createQmlWindow(const QString& qmlResource, QWindow* parent, QObject* controller)
{
    if (!mQmlEngine)
    {
        UIFabrication_LOG_WARN("no qml engine");
        return nullptr;
    }

    if (qmlResource.isEmpty())
    {
        return nullptr;
    }

    auto view = new QQuickView(mQmlEngine.get(), parent);

    if (controller)
    {
        view->setInitialProperties({ {QStringLiteral("controller"), QVariant::fromValue(controller)} });
    }

    view->setSource(generateQmlResourcePath(qmlResource));
    return view;
}


void UIViewFactory::loadQmlWindow(const QString& qmlResource, const QString& controllerObjectName, const UICore::ControllerCallback& controllerCallback)
{
    if (!mQmlEngine)
    {
        UIFabrication_LOG_WARN("no qml engine");
        return;
    }

    if (qmlResource.isEmpty())
    {
        UIFabrication_LOG_WARN("empty qmlResource url");
        return;
    }

    QString actualQmlResource = generateQmlResourcePath(qmlResource);
    
    QObject::connect(mQmlEngine.get(), &QQmlApplicationEngine::objectCreated, [actualQmlResource, controllerObjectName, controllerCallback](QObject* object, const QUrl& url) {
        if (object && url.toString() == actualQmlResource)
        {
            UIFabrication_LOG_DEBUG("object created: " << url.toString().toStdString());
            if (!controllerObjectName.isEmpty() && controllerCallback)
            {
                if (auto controller = object->findChild<UICore::CoreController*>(controllerObjectName))
                {
                    UIFabrication_LOG_DEBUG("controller found, controllerName: " << controller->getControllerName().toStdString() << ", objectName: "<< controller->objectName().toStdString());
                    controllerCallback(controller);
                }
                else
                {
                    UIFabrication_LOG_WARN("controller not found, controllerObjectName: " << controllerObjectName.toStdString());
                }
            }
        }
        else
        {
            UIFabrication_LOG_WARN("object created failed: " << actualQmlResource.toStdString());
        }
    });
    mQmlEngine->load(actualQmlResource);
}   
}