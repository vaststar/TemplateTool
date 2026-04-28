#pragma once

#include <UIFabrication/IUIViewFactory.h>

class QQuickWindow;

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
public:
    virtual QPointer<QQuickView> createQmlView(const QString& qmlResource, QWindow* parent = nullptr, QObject* controller = nullptr) override;

    virtual void loadQmlWindow(const QString& qmlResource, QWindow* parentWindow = nullptr) override;
    virtual void loadQmlWindow(const QString& qmlResource, UIAppCore::UIController* controller, QWindow* parentWindow = nullptr) override;
    virtual void loadQmlWindow(const QString& qmlResource, const UIAppCore::ControllerCallback& controllerCallback, QWindow* parentWindow = nullptr) override;
private:
    QString getQRCPrefixPath() const;
    QString generateQmlResourcePath(const QString& qmlResource) const;
    void setupChildWindow(QQuickWindow* window, QWindow* parentWindow);
private:
    const QPointer<UIAppCore::UIQmlEngine> mQmlEngine;
};
}
