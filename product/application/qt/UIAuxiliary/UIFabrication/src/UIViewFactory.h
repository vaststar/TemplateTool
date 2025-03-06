#pragma once

#include <memory>

#include <QPointer>
#include <QObject>
#include <QQuickView>
#include <UICore/CoreController.h>
#include <UIFabrication/IUIViewFactory.h>

namespace UICore{
class CoreQmlEngine;
class CoreController;
using ControllerCallback = std::function<void(CoreController*)>;
}

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

    virtual QPointer<QQuickView> createQmlWindow(const QString& qmlResource, QWindow* parent = nullptr, QObject* controller = nullptr) override;

    virtual void loadQmlWindow(const QString& qmlResource, const QString& controllerObjectName = QString(), const UICore::ControllerCallback& controllerCallback = nullptr) override;
private:
    QString getQRCPrefixPath() const;
    QString generateQmlResourcePath(const QString& qmlResource) const;
private:
    const QPointer<UICore::CoreQmlEngine> mQmlEngine;
};
}