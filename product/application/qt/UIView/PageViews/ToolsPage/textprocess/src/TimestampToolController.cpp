#include "PageViews/ToolsPage/textprocess/include/TimestampToolController.h"
#include "LoggerDefine/LoggerDefine.h"

#include <commonHead/viewModels/ToolsViewModel/IToolsViewModel.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>
#include <QClipboard>
#include <QGuiApplication>
#include <QDateTime>
#include <QTimeZone>
#include <QtMath>

TimestampToolController::TimestampToolController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create TimestampToolController");
}

TimestampToolController::~TimestampToolController() = default;

void TimestampToolController::init()
{
    UIVIEW_LOG_DEBUG("TimestampToolController::init");
    m_toolsViewModel = getAppContext()->getViewModelFactory()->createToolsViewModelInstance();
    m_toolsViewModel->initViewModel();

    initTimezoneModel();

    connect(&m_clockTimer, &QTimer::timeout, this, &TimestampToolController::updateLiveClock);
    m_clockTimer.setInterval(1000);
    m_clockTimer.start();
    updateLiveClock();
}

// ========== Timezone model ==========

void TimestampToolController::appendTimezone(const QString& label, const QTimeZone& tz)
{
    m_timezoneModel << label;
    m_timezones << tz;
}

void TimestampToolController::initTimezoneModel()
{
    const auto localTz = QTimeZone::systemTimeZone();
    const auto localOffset = localTz.displayName(QDateTime::currentDateTime(), QTimeZone::OffsetName);
    appendTimezone(tr("Local") + " (" + localOffset + ")", localTz);
    appendTimezone(QStringLiteral("UTC"), QTimeZone::utc());

    for (int h = -12; h <= 12; ++h) {
        if (h == 0) continue;                          // already added UTC
        QTimeZone tz(h * 3600);
        if (tz == localTz) continue;                   // already added local
        appendTimezone(QStringLiteral("UTC%1%2:00").arg(h > 0 ? '+' : '-').arg(qAbs(h), 2, 10, QChar('0')), tz);
    }
}

// ========== Live clock ==========

void TimestampToolController::updateLiveClock()
{
    auto now = QDateTime::currentDateTime();
    auto utcNow = now.toUTC();

    m_liveTimestamp = QString::number(now.toSecsSinceEpoch());
    m_liveUtcDateTime = utcNow.toString("yyyy-MM-dd HH:mm:ss");
    m_liveLocalDateTime = now.toString("yyyy-MM-dd HH:mm:ss");
    m_liveTimezone = now.timeZone().displayName(now, QTimeZone::OffsetName);
    emit liveClockChanged();
}

QString TimestampToolController::liveTimestamp() const { return m_liveTimestamp; }
QString TimestampToolController::liveUtcDateTime() const { return m_liveUtcDateTime; }
QString TimestampToolController::liveLocalDateTime() const { return m_liveLocalDateTime; }
QString TimestampToolController::liveTimezone() const { return m_liveTimezone; }

void TimestampToolController::copyLiveTimestamp()
{
    if (auto* clipboard = QGuiApplication::clipboard())
        clipboard->setText(m_liveTimestamp);
}

void TimestampToolController::copyText(const QString& text)
{
    if (auto* clipboard = QGuiApplication::clipboard())
        clipboard->setText(text);
}

// ========== Inputs ==========

QString TimestampToolController::getTimestampInput() const { return m_timestampInput; }

void TimestampToolController::setTimestampInput(const QString& text)
{
    if (m_timestampInput != text) {
        m_timestampInput = text;
        emit timestampInputChanged();
    }
}

QString TimestampToolController::getDateTimeInput() const { return m_dateTimeInput; }

void TimestampToolController::setDateTimeInput(const QString& text)
{
    if (m_dateTimeInput != text) {
        m_dateTimeInput = text;
        emit dateTimeInputChanged();
    }
}

bool TimestampToolController::isMilliseconds() const { return m_isMilliseconds; }

void TimestampToolController::setIsMilliseconds(bool ms)
{
    if (m_isMilliseconds != ms) {
        m_isMilliseconds = ms;
        emit isMillisecondsChanged();
    }
}

QStringList TimestampToolController::timezoneModel() const { return m_timezoneModel; }

int TimestampToolController::dtTimezoneIndex() const { return m_dtTimezoneIndex; }

void TimestampToolController::setDtTimezoneIndex(int index)
{
    if (index >= 0 && index < m_timezones.size() && m_dtTimezoneIndex != index) {
        m_dtTimezoneIndex = index;
        emit dtTimezoneIndexChanged();
        if (!m_dateTimeInput.trimmed().isEmpty())
            dateTimeToTimestamp();
    }
}

QString TimestampToolController::getErrorMessage() const { return m_errorMessage; }

// ========== Timestamp → DateTime results ==========

QString TimestampToolController::tsResultUtc() const { return m_tsResultUtc; }
QString TimestampToolController::tsResultLocal() const { return m_tsResultLocal; }
QString TimestampToolController::tsResultRelative() const { return m_tsResultRelative; }

// ========== DateTime → Timestamp results ==========

QString TimestampToolController::dtResultSeconds() const { return m_dtResultSeconds; }
QString TimestampToolController::dtResultMillis() const { return m_dtResultMillis; }

// ========== Actions ==========

static QString formatRelativeTime(const QDateTime& target)
{
    auto now = QDateTime::currentDateTime();
    qint64 diffSecs = target.secsTo(now);
    bool past = diffSecs >= 0;
    qint64 absDiff = qAbs(diffSecs);

    QString value;
    if (absDiff < 60) {
        value = QString::number(absDiff) + " " + QObject::tr("seconds");
    } else if (absDiff < 3600) {
        value = QString::number(absDiff / 60) + " " + QObject::tr("minutes");
    } else if (absDiff < 86400) {
        value = QString::number(absDiff / 3600) + " " + QObject::tr("hours");
    } else {
        value = QString::number(absDiff / 86400) + " " + QObject::tr("days");
    }
    return past ? value + " " + QObject::tr("ago") : value + " " + QObject::tr("from now");
}

void TimestampToolController::timestampToDateTime()
{
    bool ok = false;
    qint64 ts = m_timestampInput.trimmed().toLongLong(&ok);

    if (!ok) {
        m_errorMessage = tr("Invalid timestamp");
        m_tsResultUtc.clear();
        m_tsResultLocal.clear();
        m_tsResultRelative.clear();
        emit errorMessageChanged();
        emit tsResultChanged();
        return;
    }

    // Convert to msecs
    qint64 msecs = m_isMilliseconds ? ts : ts * 1000;
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(msecs);
    QDateTime utcDt = dt.toUTC();

    m_tsResultUtc = utcDt.toString("yyyy-MM-dd HH:mm:ss");
    m_tsResultLocal = dt.toString("yyyy-MM-dd HH:mm:ss") +
                      " (" + dt.timeZone().displayName(dt, QTimeZone::OffsetName) + ")";
    m_tsResultRelative = formatRelativeTime(dt);
    m_errorMessage.clear();

    emit tsResultChanged();
    emit errorMessageChanged();
}

void TimestampToolController::dateTimeToTimestamp()
{
    QString input = m_dateTimeInput.trimmed();
    QDateTime dt;

    // Try common formats
    static const QStringList formats = {
        "yyyy-MM-dd HH:mm:ss",
        "yyyy-MM-dd HH:mm",
        "yyyy-MM-dd",
        "yyyy/MM/dd HH:mm:ss",
        "yyyy/MM/dd HH:mm",
        "yyyy/MM/dd",
    };

    for (const auto& fmt : formats) {
        dt = QDateTime::fromString(input, fmt);
        if (dt.isValid()) break;
    }

    if (dt.isValid() && m_dtTimezoneIndex >= 0 && m_dtTimezoneIndex < m_timezones.size()) {
        dt.setTimeZone(m_timezones[m_dtTimezoneIndex]);
    }

    if (!dt.isValid()) {
        m_errorMessage = tr("Cannot parse datetime, supported format: YYYY-MM-DD HH:MM:SS");
        m_dtResultSeconds.clear();
        m_dtResultMillis.clear();
        emit errorMessageChanged();
        emit dtResultChanged();
        return;
    }

    qint64 secs = dt.toSecsSinceEpoch();
    qint64 msecs = dt.toMSecsSinceEpoch();

    m_dtResultSeconds = QString::number(secs);
    m_dtResultMillis = QString::number(msecs);
    m_errorMessage.clear();

    emit dtResultChanged();
    emit errorMessageChanged();
}
