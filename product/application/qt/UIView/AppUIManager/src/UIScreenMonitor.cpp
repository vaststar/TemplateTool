#include "UIScreenMonitor.h"

#include <string>

#include <QGuiApplication>
#include <QScreen>
#include <QTimer>

#include <AppContext/AppContext.h>
#include <UIManager/IUIManagerProvider.h>
#include <UIEventBus/IUIEventBus.h>

#include "UIEvents/UIScreenChangedEvent.h"
#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

namespace
{
// One-line description of a screen's current geometry/DPI for logging.
std::string describeScreen(const QScreen* screen)
{
    if (!screen)
    {
        return "<null>";
    }
    const QRect g = screen->geometry();
    const QRect a = screen->availableGeometry();
    const QString s =
        QStringLiteral("'%1' geo=%2x%3@(%4,%5) avail=%6x%7@(%8,%9) logicalDpi=%10 dpr=%11")
            .arg(screen->name())
            .arg(g.width()).arg(g.height()).arg(g.x()).arg(g.y())
            .arg(a.width()).arg(a.height()).arg(a.x()).arg(a.y())
            .arg(screen->logicalDotsPerInch(), 0, 'f', 1)
            .arg(screen->devicePixelRatio(), 0, 'f', 2);
    return s.toStdString();
}
} // namespace

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
    UIVIEW_LOG_DEBUG("initial screen count=" << static_cast<int>(screens.size()));
    for (QScreen* screen : screens)
    {
        UIVIEW_LOG_DEBUG("  screen " << describeScreen(screen));
        connectScreen(screen);
    }

    connect(qApp, &QGuiApplication::screenAdded, this, [this](QScreen* screen) {
        UIVIEW_LOG_DEBUG("screenAdded: " << describeScreen(screen));
        connectScreen(screen);
        scheduleNotify();
    });
    connect(qApp, &QGuiApplication::screenRemoved, this, [this](QScreen* screen) {
        UIVIEW_LOG_DEBUG("screenRemoved: " << (screen ? screen->name().toStdString() : std::string("<null>")));
        scheduleNotify();
    });
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, [this](QScreen* screen) {
        UIVIEW_LOG_DEBUG("primaryScreenChanged: " << describeScreen(screen));
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
    connect(screen, &QScreen::availableGeometryChanged, this, [this, screen](const QRect& g) {
        UIVIEW_LOG_DEBUG("availableGeometryChanged '" << screen->name().toStdString() << "' -> "
                         << g.width() << "x" << g.height() << "@(" << g.x() << "," << g.y() << ")");
        scheduleNotify();
    });
    connect(screen, &QScreen::geometryChanged, this, [this, screen](const QRect& g) {
        UIVIEW_LOG_DEBUG("geometryChanged '" << screen->name().toStdString() << "' -> "
                         << g.width() << "x" << g.height() << "@(" << g.x() << "," << g.y() << ")");
        scheduleNotify();
    });
    connect(screen, &QScreen::logicalDotsPerInchChanged, this, [this, screen](qreal dpi) {
        UIVIEW_LOG_DEBUG("logicalDotsPerInchChanged '" << screen->name().toStdString() << "' -> "
                         << dpi << " (dpr=" << screen->devicePixelRatio() << ")");
        scheduleNotify();
    });
    connect(screen, &QScreen::physicalDotsPerInchChanged, this, [this, screen](qreal dpi) {
        UIVIEW_LOG_DEBUG("physicalDotsPerInchChanged '" << screen->name().toStdString() << "' -> " << dpi);
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
    const auto screens = QGuiApplication::screens();
    UIVIEW_LOG_DEBUG("current screen count=" << static_cast<int>(screens.size())
                     << ", primary=" << describeScreen(QGuiApplication::primaryScreen()));
    for (QScreen* screen : screens)
    {
        UIVIEW_LOG_DEBUG("  screen " << describeScreen(screen));
    }

    UIScreenChangedEvent event;
    bus->send(&event);
}
