#pragma once

#include <QObject>
#include <QPointer>

#include <UIAppCore/UIController.h>
#include <UIEventBus/IUIEventBus.h>

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

    /// Access the application UIEventBus. Returns nullptr before initializeController().
    QPointer<UIManager::IUIEventBus> getUIEventBus() const;

    /// Convenience: construct and send an event in one call.
    template <typename EventT, typename... Args>
    void sendUIEvent(Args&&... args)
    {
        if (auto bus = getUIEventBus()) {
            EventT event(std::forward<Args>(args)...);
            bus->send(&event);
        }
    }

    /// Convenience: register this controller as listener for one or more event types.
    /// Call in init(). Automatically removed when this controller is destroyed.
    template <typename... Events>
    void listenUIEvents()
    {
        if (auto bus = getUIEventBus()) {
            bus->addListeners<Events...>(this);
        }
    }

signals:
    void controllerInitialized();
    void languageChanged();
    void themeChanged();
private:
    QPointer<AppContext> mAppContext;
};
