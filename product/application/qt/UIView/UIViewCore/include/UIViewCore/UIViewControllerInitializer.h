#pragma once

#include <QObject>
#include <QPointer>
#include <QtQml>

#include <UIViewCore/UIViewCoreExport.h>

class AppContext;
class UIViewController;

class UIViewCore_EXPORT UIViewControllerInitializer : public QObject
{
    Q_OBJECT
    QML_UNCREATABLE("Cannot create UIViewControllerInitializer in QML")
public:
    explicit UIViewControllerInitializer(QPointer<AppContext> appContext, QObject* parent = nullptr);
    Q_INVOKABLE void initializeController(UIViewController* controller);

private:
    QPointer<AppContext> mAppContext;
};
