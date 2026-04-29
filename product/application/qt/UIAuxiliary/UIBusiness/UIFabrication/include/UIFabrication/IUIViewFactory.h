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

    static std::unique_ptr<IUIViewFactory> createInstance(QPointer<UIAppCore::UIQmlEngine> qmlEngine);
};
}
