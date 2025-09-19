#pragma once

#include <QObject>
#include <QPointer>
#include <QtQml>

#include <UICore/CoreController.h>

class AppContext;
class UIViewController;
class UIViewControllerInitializer: public QObject
{
    Q_OBJECT
    QML_UNCREATABLE("Cannot create UIViewControllerInitializer in QML")
public:
    UIViewControllerInitializer(const QPointer<AppContext>& appContext, QObject* parent = nullptr);
    void initializeController(const QPointer<UIViewController>& controller);
private:
    QPointer<AppContext> mAppContext;
};