#pragma once

#include <QObject>
#include <QPointer>

#include <UICore/CoreController.h>

class AppContext;
class UIViewController : public UICore::CoreController
{
    Q_OBJECT
public:
    UIViewController(QObject* parent = nullptr);
    virtual ~UIViewController() = default;

    virtual void initializeController(const QPointer<AppContext>& appContext) final;
    
    Q_INVOKABLE void logInfo(const QString& message);
protected:
    virtual void init() = 0;
    QPointer<AppContext> getAppContext() const;
private:
    QPointer<AppContext> mAppContext;
};