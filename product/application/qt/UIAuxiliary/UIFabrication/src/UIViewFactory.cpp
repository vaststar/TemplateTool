#include "UIViewFactory.h"
#include <UICore/CoreQmlEngine.h>
#include <UICore/CoreController.h>

#include <QFileInfo>
#include <QApplication>
#include <QQmlComponent>

#include "LoggerDefine.h"


namespace UIFabrication{
std::unique_ptr<IUIViewFactory> IUIViewFactory::createInstance(QPointer<UICore::CoreQmlEngine> qmlEngine)
{
    return std::make_unique<UIViewFactory>(qmlEngine);
}

UIViewFactory::UIViewFactory(QPointer<UICore::CoreQmlEngine> qmlEngine)
    : mQmlEngine(qmlEngine)
{
    if (mQmlEngine)
    {
        for (auto path: mQmlEngine->importPathList())
        {
            UIFabrication_LOG_DEBUG("QML Import Paths:" << path.toStdString());
        }
    }
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

QPointer<QQuickView> UIViewFactory::createQmlView(const QString& qmlResource, QWindow* parent, QObject* controller)
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

void UIViewFactory::loadQmlWindow(const QString& qmlResource)
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
    if (QQmlComponent component(mQmlEngine.get(), actualQmlResource); component.status() == QQmlComponent::Ready)
    {
        if (auto object = component.create())
        {
            UIFabrication_LOG_DEBUG("load qml succeed: " << actualQmlResource.toStdString() << ", objectName: "<< object->objectName().toStdString());
            if (QQuickWindow* window = qobject_cast<QQuickWindow*>(object))
            {
                QObject::connect(window, &QQuickWindow::closing, [window]{ 
                    if (window)
                    {
                        UIFabrication_LOG_DEBUG("will close window: " << window->objectName().toStdString());
                        window->deleteLater();
                    }
                });
            }
        }
        else
        {
            UIFabrication_LOG_WARN("load qml failed: " << actualQmlResource.toStdString() << ", error: " << component.errorString().toStdString());
        }
    }
    else
    {
        UIFabrication_LOG_WARN("load qml failed: " << actualQmlResource.toStdString() << ", error: " << component.errorString().toStdString());
    }

    // QString actualQmlResource = generateQmlResourcePath(qmlResource);
    // UIFabrication_LOG_DEBUG("start load qml: " << actualQmlResource.toStdString());
    // mQmlEngine->load(actualQmlResource);
    // UIFabrication_LOG_DEBUG("finish load qml: " << actualQmlResource.toStdString());
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
    if (QQmlComponent component(mQmlEngine.get(), actualQmlResource); component.status() == QQmlComponent::Ready)
    {
        if (auto object = component.create())
        {
            UIFabrication_LOG_DEBUG("create qml succeed: " << actualQmlResource.toStdString() << ", objectName: "<< object->objectName().toStdString());
            if (QQuickWindow* window = qobject_cast<QQuickWindow*>(object))
            {
                QObject::connect(window, &QQuickWindow::closing, [window]{ 
                    if (window)
                    {
                        UIFabrication_LOG_DEBUG("will close window: " << window->objectName().toStdString());
                        window->deleteLater();
                    }
                });
            }

            if (!controllerObjectName.isEmpty() && controllerCallback)
            {
                if (auto controller = object->findChild<UICore::CoreController*>(controllerObjectName))
                {
                    UIFabrication_LOG_DEBUG("controller found, do callback, controllerName: " << controller->getControllerName().toStdString() << ", objectName: "<< controller->objectName().toStdString());
                    controllerCallback(controller);
                    UIFabrication_LOG_DEBUG("controller found, callback done, controllerName: " << controller->getControllerName().toStdString() << ", objectName: "<< controller->objectName().toStdString());
                }
                else
                {
                    UIFabrication_LOG_WARN("controller not found, controllerObjectName: " << controllerObjectName.toStdString());
                }
            }
        }
        else
        {
            UIFabrication_LOG_WARN("create qml failed: " << actualQmlResource.toStdString() << ", error: " << component.errorString().toStdString());
        }
    }
    else
    {
        UIFabrication_LOG_WARN("load qml failed: " << actualQmlResource.toStdString() << ", error: " << component.errorString().toStdString());
    }
}  

void UIViewFactory::loadQmlWindow(const QString& qmlResource, UICore::CoreController* controller)
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
    if (QQmlComponent component(mQmlEngine.get(), actualQmlResource); component.status() == QQmlComponent::Ready)
    {
        if (!controller)
        {
            UIFabrication_LOG_WARN("no controller, qmlResource: " << actualQmlResource.toStdString());
            return;
        }
        if (auto object = component.createWithInitialProperties({{QStringLiteral("controller"), QVariant::fromValue(controller)}}))
        {
            UIFabrication_LOG_DEBUG("create qml succeed: " << actualQmlResource.toStdString() << ", objectName: "<< object->objectName().toStdString() << ", controllerName: " << controller->getControllerName().toStdString());
            if (QQuickWindow* window = qobject_cast<QQuickWindow*>(object))
            {
                QObject::connect(window, &QQuickWindow::closing, [window]{ 
                    if (window)
                    {
                        UIFabrication_LOG_DEBUG("will close window: " << window->objectName().toStdString());
                        window->deleteLater();
                    }
                });
            }
        }
        else
        {
            UIFabrication_LOG_WARN("create qml failed: " << actualQmlResource.toStdString() << ", error: " << component.errorString().toStdString());
        }
    }
    else
    {
        UIFabrication_LOG_WARN("load qml failed: " << actualQmlResource.toStdString() << ", error: " << component.errorString().toStdString());
    }
}
}