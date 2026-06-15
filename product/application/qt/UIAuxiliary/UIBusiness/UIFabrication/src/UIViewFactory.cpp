#include "UIViewFactory.h"
#include <UIAppCore/UIQmlEngine.h>

#include <QQmlComponent>
#include <QQuickItem>
#include <QQuickView>
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

QPointer<QQuickWindow> UIViewFactory::createQmlItemWindow(const QString& qmlResource,
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

    auto* view = new QQuickView(mQmlEngine.get(), /*parent*/ nullptr);
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    if (!initialProperties.isEmpty())
    {
        view->setInitialProperties(initialProperties);
    }
    view->setSource(QUrl(actualQmlResource));

    if (view->status() != QQuickView::Ready)
    {
        QString errs;
        for (const auto& e : view->errors())
        {
            errs += e.toString() + QStringLiteral("; ");
        }
        UIFabrication_LOG_WARN("load qml failed: " << actualQmlResource.toStdString()
                              << ", error: " << errs.toStdString());
        delete view;
        return nullptr;
    }

    QQuickItem* root = view->rootObject();
    if (!root)
    {
        UIFabrication_LOG_WARN("qml root is null: " << actualQmlResource.toStdString());
        delete view;
        return nullptr;
    }
    // Defensive: a Window-as-root would have been rejected by QQuickView
    // already (status != Ready), but the explicit check keeps the contract
    // visible in the logs.
    if (qobject_cast<QQuickWindow*>(root))
    {
        UIFabrication_LOG_WARN("qml root is a Window; use createQmlWindow() instead: "
                              << actualQmlResource.toStdString());
        delete view;
        return nullptr;
    }

    UIFabrication_LOG_DEBUG("item-window created: " << actualQmlResource.toStdString()
                          << ", rootObjectName: " << root->objectName().toStdString());

    // SizeRootObjectToView lets the root Item follow the window on resize/maximize,
    // but no longer adopts the root's implicit size, so seed the initial window size.
    const int implicitW = static_cast<int>(root->implicitWidth());
    const int implicitH = static_cast<int>(root->implicitHeight());
    if (implicitW > 0 && implicitH > 0)
    {
        view->resize(implicitW, implicitH);
    }

    installCloseHandler(view);
    return view;
}

}
