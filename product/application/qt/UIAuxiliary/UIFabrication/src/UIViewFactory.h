#pragma once

#include <UIFabrication/IUIViewFactory.h>

namespace UIFabrication{
class UIViewFactory final: public IUIViewFactory
{
Q_OBJECT
public:
    explicit UIViewFactory(QPointer<UICore::CoreQmlEngine> qmlEngine);
    UIViewFactory(const UIViewFactory&) = delete;
    UIViewFactory(UIViewFactory&&) = delete;
    UIViewFactory& operator=(const UIViewFactory&) = delete;
    UIViewFactory& operator=(UIViewFactory&&) = delete;
    virtual ~UIViewFactory();
public:
    virtual QPointer<QQuickView> createQmlView(const QString& qmlResource, QWindow* parent = nullptr, QObject* controller = nullptr) override;

    virtual void loadQmlWindow(const QString& qmlResource) override;
    virtual void loadQmlWindow(const QString& qmlResource, UICore::CoreController* controller) override;
    virtual void loadQmlWindow(const QString& qmlResource, const UICore::ControllerCallback& controllerCallback) override;
private:
    QString getQRCPrefixPath() const;
    QString generateQmlResourcePath(const QString& qmlResource) const;
private:
    const QPointer<UICore::CoreQmlEngine> mQmlEngine;
};
}