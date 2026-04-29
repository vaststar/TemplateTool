#include "UIViewFactory.h"
#include <UIAppCore/UIQmlEngine.h>

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

void UIViewFactory::installCloseHandler(QQuickWindow* window)
{
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
    return getQRCPrefixPath() + qmlResource;
}

QPointer<QQuickWindow> UIViewFactory::createQmlWindow(const QString& qmlResource,
                                                     const QVariantMap& initialProperties)
{
    if (!mQmlEngine)
    {
        UIFabrication_LOG_WARN("no qml engine");
        return nullptr;
    }
    if (qmlResource.isEmpty())
    {
        UIFabrication_LOG_WARN("empty qmlResource url");
        return nullptr;
    }

    const QString actualQmlResource = generateQmlResourcePath(qmlResource);
    QQmlComponent component(mQmlEngine.get(), actualQmlResource);
    if (component.status() != QQmlComponent::Ready)
    {
        UIFabrication_LOG_WARN("load qml failed: " << actualQmlResource.toStdString()
                              << ", error: " << component.errorString().toStdString());
        return nullptr;
    }

    QObject* object = initialProperties.isEmpty()
        ? component.create()
        : component.createWithInitialProperties(initialProperties);
    if (!object)
    {
        UIFabrication_LOG_WARN("create qml failed: " << actualQmlResource.toStdString()
                              << ", error: " << component.errorString().toStdString());
        return nullptr;
    }

    auto* window = qobject_cast<QQuickWindow*>(object);
    if (!window)
    {
        UIFabrication_LOG_WARN("qml root is not a Window: " << actualQmlResource.toStdString()
                              << ", objectName: " << object->objectName().toStdString());
        // Avoid leaking the non-window root by handing ownership to the engine.
        object->setParent(mQmlEngine.get());
        return nullptr;
    }

    UIFabrication_LOG_DEBUG("window created: " << actualQmlResource.toStdString()
                          << ", objectName: " << window->objectName().toStdString());
    installCloseHandler(window);
    return window;
}

}
