#pragma once

#include <memory>

#include <QPointer>
#include <QObject>
#include <QQuickWindow>
#include <QVariantMap>

#include <UIFabrication/UIFabricationExport.h>

namespace UIAppCore{
class UIQmlEngine;
}

namespace UIFabrication{
class UIFabrication_EXPORT IUIViewFactory: public QObject
{
Q_OBJECT
public:
    virtual ~IUIViewFactory() = default;

    /**
     * @brief Create a top-level QML window and install the standard
     *        closing -> deleteLater handler.
     *
     * @param qmlResource        QML resource path, e.g.
     *                           "UIView/AppUpgrade/qml/UpgradeDialog.qml",
     *                           or a full "qrc:/..." URL.
     * @param initialProperties  Properties injected at construction time via
     *                           QQmlComponent::createWithInitialProperties().
     *                           Visible inside QML's Component.onCompleted, so
     *                           use this for `required property` bindings such
     *                           as `controller`.
     * @return The created window; null on failure.
     *
     * @note Only local / qrc resources are supported (synchronous load).
     * @note Does NOT call show(), does NOT set transient parent and does NOT
     *       set window position. Callers are responsible for those concerns
     *       (e.g. via UIView::UIViewHelper).
     */
    virtual QPointer<QQuickWindow> createQmlWindow(const QString& qmlResource,
                                                   const QVariantMap& initialProperties = {}) = 0;

    /**
     * @brief Wrap a QML whose root is a QQuickItem (NOT a Window) into a
     *        top-level QQuickWindow via QQuickView, and install the standard
     *        closing -> deleteLater handler.
     *
     * @param qmlResource        QML resource path, e.g.
     *                           "UIView/Foo/qml/FooPanel.qml",
     *                           or a full "qrc:/..." URL. Root MUST be Item.
     * @param initialProperties  Properties injected at construction time (Qt
     *                           5.14+ QQuickView::setInitialProperties()),
     *                           visible inside the Item's
     *                           Component.onCompleted; use this for
     *                           `required property` bindings such as
     *                           `controller`.
     * @return The created window (actually a QQuickView, exposed as
     *         QQuickWindow); null on failure or if the QML root is a Window
     *         (use createQmlWindow() for that case).
     *
     * @note The returned window uses QQuickView::SizeViewToRootObject: the
     *       window size follows the root Item's implicit / explicit size.
     * @note Only local / qrc resources are supported (synchronous load).
     * @note Does NOT call show(), does NOT set transient parent, does NOT
     *       set window position and does NOT touch window flags. The window
     *       therefore inherits the platform's default decorations
     *       (title bar, min/close buttons). Callers wanting a frameless
     *       look must call setFlags(Qt::FramelessWindowHint) themselves.
     * @note To expose a controller to UIView::UIViewHelper::controllerOf(),
     *       declare `controller` as a property of the root Item (mirroring
     *       the Window-as-root convention).
     */
    virtual QPointer<QQuickWindow> createQmlItemWindow(const QString& qmlResource,
                                                       const QVariantMap& initialProperties = {}) = 0;

    static std::unique_ptr<IUIViewFactory> createInstance(QPointer<UIAppCore::UIQmlEngine> qmlEngine);
};
}
