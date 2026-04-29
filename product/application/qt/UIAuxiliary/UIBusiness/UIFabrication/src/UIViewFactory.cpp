#include "UIViewFactory.h"
#include <UIAppCore/UIQmlEngine.h>
#include <UIAppCore/UIController.h>

#include <QFileInfo>
#include <QApplication>
#include <QQmlComponent>
#include <QQuickWindow>

#include "LoggerDefine.h"

namespace UIFabrication{
std::unique_ptr<IUIViewFactory> IUIViewFactory::createInstance(QPointer<UIAppCore::UIQmlEngine> qmlEngine)
{
    return std::make_unique<UIViewFactory>(qmlEngine);
}

UIViewFactory::UIViewFactory(QPointer<UIAppCore::UIQmlEngine> qmlEngine)
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

void UIViewFactory::setupChildWindow(QQuickWindow* window, QWindow* parentWindow)
{
    if (!parentWindow)
    {
        for (QWindow* tlw : QGuiApplication::topLevelWindows())
        {
            if (tlw != window && tlw->isVisible())
            {
                parentWindow = tlw;
                break;
            }
        }
    }
    if (parentWindow)
    {
        window->setTransientParent(parentWindow);

        // Centering must happen AFTER the child window has its real size.
        // At setup time on Linux/X11 the QML-declared width/height may not
        // be applied yet (window->width() returns the default), and many
        // window managers ignore setPosition() on a not-yet-mapped window.
        // Hook visibleChanged: when the window first becomes visible Qt has
        // resolved the QML size, so centering math is correct.
        auto centerOnce = std::make_shared<QMetaObject::Connection>();
        QPointer<QWindow> parentGuard(parentWindow);
        QPointer<QQuickWindow> windowGuard(window);
        *centerOnce = QObject::connect(window, &QWindow::visibleChanged,
            [windowGuard, parentGuard, centerOnce](bool visible)
        {
            if (!visible)
            {
                // Wait for the first visible=true; ignore intermediate hides.
                return;
            }
            // We only want to center once on first show, so disconnect first
            // (also releases the shared_ptr captured in this lambda).
            QObject::disconnect(*centerOnce);
            if (!windowGuard || !parentGuard)
            {
                return;
            }
            const auto parentGeo = parentGuard->geometry();
            const int x = parentGeo.x() + (parentGeo.width()  - windowGuard->width())  / 2;
            const int y = parentGeo.y() + (parentGeo.height() - windowGuard->height()) / 2;
            windowGuard->setPosition(x, y);
        });
    }
    QObject::connect(window, &QQuickWindow::closing, [window]{
        if (window)
        {
            UIFabrication_LOG_DEBUG("will close window: " << window->objectName().toStdString());
            window->deleteLater();
        }
    });
}

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

void UIViewFactory::loadQmlWindow(const QString& qmlResource, QWindow* parentWindow)
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
                UIFabrication_LOG_DEBUG("window created, install closing handler, qmlResource: " << actualQmlResource.toStdString() << ", window objectName: " << window->objectName().toStdString());
                setupChildWindow(window, parentWindow);
            }
            else
            {
                UIFabrication_LOG_DEBUG("non-window qml object created, set parent to qml engine, qmlResource: " << actualQmlResource.toStdString() << ", objectName: " << object->objectName().toStdString());
                object->setParent(mQmlEngine.get());
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
}

void UIViewFactory::loadQmlWindow(const QString& qmlResource, const UIAppCore::ControllerCallback& controllerCallback, QWindow* parentWindow)
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
                UIFabrication_LOG_DEBUG("window created, install closing handler, qmlResource: " << actualQmlResource.toStdString() << ", window objectName: " << window->objectName().toStdString());
                setupChildWindow(window, parentWindow);
            }
            else
            {
                UIFabrication_LOG_DEBUG("non-window qml object created, set parent to qml engine, qmlResource: " << actualQmlResource.toStdString() << ", objectName: " << object->objectName().toStdString());
                object->setParent(mQmlEngine.get());
            }

            if (controllerCallback)
            {
                if (QVariant controllerVar = object->property("controller"); controllerVar.isValid())
                {
                    if (auto controller = qobject_cast<UIAppCore::UIController*>(controllerVar.value<QObject*>()))
                    {
                        UIFabrication_LOG_DEBUG("controller found, do callback, controllerName: " << controller->getControllerName().toStdString() << ", objectName: "<< controller->objectName().toStdString());
                        controllerCallback(controller);
                        UIFabrication_LOG_DEBUG("controller found, callback done, controllerName: " << controller->getControllerName().toStdString() << ", objectName: "<< controller->objectName().toStdString());
                    }
                    else
                    {
                        UIFabrication_LOG_WARN("no controller object found in qml object, objectName: " << object->objectName().toStdString());
                    }
                }
                else
                {
                    UIFabrication_LOG_WARN("no controller property found in qml object, objectName: " << object->objectName().toStdString());
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

void UIViewFactory::loadQmlWindow(const QString& qmlResource, UIAppCore::UIController* controller, QWindow* parentWindow)
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
                UIFabrication_LOG_DEBUG("window created, install closing handler, qmlResource: " << actualQmlResource.toStdString() << ", window objectName: " << window->objectName().toStdString());
                setupChildWindow(window, parentWindow);
            }
            else
            {
                UIFabrication_LOG_DEBUG("non-window qml object created, set parent to qml engine, qmlResource: " << actualQmlResource.toStdString() << ", objectName: " << object->objectName().toStdString());
                object->setParent(mQmlEngine.get());
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
