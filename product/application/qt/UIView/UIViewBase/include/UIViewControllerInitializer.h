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
    UIViewControllerInitializer(QPointer<AppContext> appContext, QObject* parent = nullptr);
    Q_INVOKABLE void initializeController(UIViewController* controller);
private:
    QPointer<AppContext> mAppContext;
};