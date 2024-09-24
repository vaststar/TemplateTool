#pragma once

#include <memory>

#include <QPointer>
#include <QObject>
#include <QQuickView>
#include <QVariant>
#include <UICore/UICoreExport.h>

class CoreQmlApplicationEngine;
class CoreController;
using ControllerCallback = std::function<void(CoreController*)>;
class UICore_EXPORT CoreViewFactory final
{
public:
    explicit CoreViewFactory(std::unique_ptr<CoreQmlApplicationEngine>&& qmlEngine);
    
    ~CoreViewFactory();

    QPointer<QQuickView> createQmlWindow(const QString& qmlResource, QWindow* parent = nullptr, QObject* controller = nullptr);

    void loadQmlWindow(const QString& qmlResource, const QString& controllerObjectName = QString(), const ControllerCallback& controllerCallback = nullptr);
private:
    const std::unique_ptr<CoreQmlApplicationEngine> mQmlEngine;
};