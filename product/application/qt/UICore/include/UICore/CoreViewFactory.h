#pragma once

#include <memory>

#include <QPointer>
#include <QObject>
#include <QQuickView>
#include <UICore/UICoreExport.h>

class CoreQmlApplicationEngine;
class UICore_EXPORT CoreViewFactory final
{
public:
    explicit CoreViewFactory(std::unique_ptr<CoreQmlApplicationEngine>&& qmlEngine);
    ~CoreViewFactory();

    QPointer<QQuickView> createQmlWindow(const QString& qmlResource, QWindow* parent = nullptr, QObject* controller = nullptr);

    void loadQmlWindow(const QString& qmlResource);
private:
    const std::unique_ptr<CoreQmlApplicationEngine> mQmlEngine;
};