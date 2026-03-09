#pragma once

#include <QObject>
#include <QPointer>

#include <UIAppCore/UIController.h>

class AppContext;
class UIViewController : public UIAppCore::UIController
{
    Q_OBJECT
public:
    UIViewController(QObject* parent = nullptr);
    virtual ~UIViewController() = default;

    void initializeController(QPointer<AppContext> appContext);
    
    // Setup another controller (inject appContext and call its initializeController)
    Q_INVOKABLE void setupController(UIViewController* controller);
    
    Q_INVOKABLE void logInfo(const QString& message);
protected:
    virtual void init() = 0;
    
    // Override this to add custom logic before controller initialization
    virtual void onSetupController(UIViewController* controller);
    
    QPointer<AppContext> getAppContext() const;
signals:
    void controllerInitialized();
private:
    QPointer<AppContext> mAppContext;
};