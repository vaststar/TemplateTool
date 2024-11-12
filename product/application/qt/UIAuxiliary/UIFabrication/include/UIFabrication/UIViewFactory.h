#pragma once

#include <memory>

#include <QPointer>
#include <QObject>
#include <QQuickView>

#include <UIFabrication/UIFabricationExport.h>

namespace UICore{
class CoreQmlEngine;
class CoreController;
using ControllerCallback = std::function<void(CoreController*)>;
}

namespace UIFabrication{
class UIFabrication_EXPORT UIViewFactory final: public QObject
{
Q_OBJECT
public:
    explicit UIViewFactory(UICore::CoreQmlEngine* qmlEngine);
    UIViewFactory(const UIViewFactory&) = delete;
    UIViewFactory(UIViewFactory&&) = delete;
    UIViewFactory& operator=(const UIViewFactory&) = delete;
    UIViewFactory& operator=(UIViewFactory&&) = delete;
    ~UIViewFactory();

    QPointer<QQuickView> createQmlWindow(const QString& qmlResource, QWindow* parent = nullptr, QObject* controller = nullptr);

    void loadQmlWindow(const QString& qmlResource, const QString& controllerObjectName = QString(), const UICore::ControllerCallback& controllerCallback = nullptr);
private:
    QString getQRCPrefixPath() const;
    QString generateQmlResourcePath(const QString& qmlResource) const;
private:
    const QPointer<UICore::CoreQmlEngine> mQmlEngine;
};
}