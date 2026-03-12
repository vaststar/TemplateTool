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
    
    // Called when application language changes. Override to refresh localized data.
    virtual void onLanguageChanged();

    // Called when application theme changes. Override to refresh theme-dependent data.
    virtual void onThemeChanged();
    
    // Override this to add custom logic before controller initialization
    virtual void onSetupController(UIViewController* controller);
    
    QPointer<AppContext> getAppContext() const;
signals:
    void controllerInitialized();
    void languageChanged();
    void themeChanged();
private:
    QPointer<AppContext> mAppContext;
};