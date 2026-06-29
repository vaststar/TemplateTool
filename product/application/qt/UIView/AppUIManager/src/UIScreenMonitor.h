#pragma once

#include <QObject>
#include <QPointer>

class AppContext;
class QTimer;
class QScreen;

// Watches screen config changes and broadcasts a debounced UIScreenChangedEvent.
class UIScreenMonitor : public QObject
{
    Q_OBJECT
public:
    explicit UIScreenMonitor(AppContext* appContext, QObject* parent = nullptr);
    ~UIScreenMonitor() override;

    void start();
    void stop();

private:
    void connectScreen(QScreen* screen);
    void scheduleNotify();
    void notifyScreenChanged();

    const QPointer<AppContext> mAppContext;
    QTimer* mDebounceTimer = nullptr;
};
