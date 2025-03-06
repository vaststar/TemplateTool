#pragma once

#include <memory>

#include <QPointer>
#include <QObject>
#include <QQuickView>
#include <UICore/CoreController.h>

#include <UIFabrication/UIFabricationExport.h>

namespace UICore{
class CoreQmlEngine;
class CoreController;
using ControllerCallback = std::function<void(CoreController*)>;
}

namespace UIFabrication{
class UIFabrication_EXPORT IUIViewFactory: public QObject
{
Q_OBJECT
public:
    virtual ~IUIViewFactory() = default;

    virtual QPointer<QQuickView> createQmlWindow(const QString& qmlResource, QWindow* parent = nullptr, QObject* controller = nullptr) = 0;

    virtual void loadQmlWindow(const QString& qmlResource, const QString& controllerObjectName = QString(), const UICore::ControllerCallback& controllerCallback = nullptr) = 0;

    static std::unique_ptr<IUIViewFactory> createInstance(QPointer<UICore::CoreQmlEngine> qmlEngine);
};
}