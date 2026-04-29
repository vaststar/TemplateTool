#pragma once

#include <UIFabrication/IUIViewFactory.h>

namespace UIAppCore{
class UIQmlEngine;
}

namespace UIFabrication{
class UIViewFactory final: public IUIViewFactory
{
Q_OBJECT
public:
    explicit UIViewFactory(QPointer<UIAppCore::UIQmlEngine> qmlEngine);
    UIViewFactory(const UIViewFactory&) = delete;
    UIViewFactory(UIViewFactory&&) = delete;
    UIViewFactory& operator=(const UIViewFactory&) = delete;
    UIViewFactory& operator=(UIViewFactory&&) = delete;
    virtual ~UIViewFactory();

    QPointer<QQuickWindow> createQmlWindow(const QString& qmlResource,
                                           const QVariantMap& initialProperties = {}) override;

private:
    QString getQRCPrefixPath() const;
    QString generateQmlResourcePath(const QString& qmlResource) const;

    // Wires the window's closing -> deleteLater handler. Does NOT touch
    // transient parent and does NOT touch window position.
    void installCloseHandler(QQuickWindow* window);

    const QPointer<UIAppCore::UIQmlEngine> mQmlEngine;
};
}
