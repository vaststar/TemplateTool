#include "UIScreenMonitor.h"

#include <QGuiApplication>
#include <QScreen>
#include <QTimer>

#include <AppContext/AppContext.h>
#include <UIManager/IUIManagerProvider.h>
#include <UIEventBus/IUIEventBus.h>

#include "UIEvents/UIScreenChangedEvent.h"
#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

UIScreenMonitor::UIScreenMonitor(AppContext* appContext, QObject* parent)
    : QObject(parent)
    , mAppContext(appContext)
{
}

UIScreenMonitor::~UIScreenMonitor()
{
    stop();
}

void UIScreenMonitor::start()
{
    UIVIEW_LOG_DEBUG("start screen monitor");

    // Coalesce a burst of screen signals into one event.
    mDebounceTimer = new QTimer(this);
    mDebounceTimer->setSingleShot(true);
    mDebounceTimer->setInterval(0);
    connect(mDebounceTimer, &QTimer::timeout, this, &UIScreenMonitor::notifyScreenChanged);

    const auto screens = QGuiApplication::screens();
    for (QScreen* screen : screens)
    {
        connectScreen(screen);
    }

    connect(qApp, &QGuiApplication::screenAdded, this, [this](QScreen* screen) {
        connectScreen(screen);
        scheduleNotify();
    });
    connect(qApp, &QGuiApplication::screenRemoved, this, [this](QScreen*) {
        scheduleNotify();
    });
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, [this](QScreen*) {
        scheduleNotify();
    });
}

void UIScreenMonitor::stop()
{
    if (mDebounceTimer)
    {
        mDebounceTimer->stop();
        mDebounceTimer = nullptr;
    }
}

void UIScreenMonitor::connectScreen(QScreen* screen)
{
    if (!screen)
    {
        return;
    }
    connect(screen, &QScreen::availableGeometryChanged, this, [this](const QRect&) {
        scheduleNotify();
    });
    connect(screen, &QScreen::geometryChanged, this, [this](const QRect&) {
        scheduleNotify();
    });
    connect(screen, &QScreen::logicalDotsPerInchChanged, this, [this](qreal) {
        scheduleNotify();
    });
    connect(screen, &QScreen::physicalDotsPerInchChanged, this, [this](qreal) {
        scheduleNotify();
    });
}

void UIScreenMonitor::scheduleNotify()
{
    if (mDebounceTimer && !mDebounceTimer->isActive())
    {
        mDebounceTimer->start();
    }
}

void UIScreenMonitor::notifyScreenChanged()
{
    if (!mAppContext)
    {
        return;
    }
    auto provider = mAppContext->getManagerProvider();
    if (!provider)
    {
        return;
    }
    auto bus = provider->getUIEventBus();
    if (!bus)
    {
        return;
    }

    UIVIEW_LOG_DEBUG("screen configuration changed, broadcasting UIScreenChangedEvent");
    UIScreenChangedEvent event;
    bus->send(&event);
}
