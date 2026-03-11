#pragma once

#include <QObject>
#include <QTimer>
#include <QTimeZone>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"

namespace commonHead::viewModels {
    class IToolsViewModel;
}

class TimestampToolController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

    // Live clock
    Q_PROPERTY(QString liveTimestamp READ liveTimestamp NOTIFY liveClockChanged)
    Q_PROPERTY(QString liveUtcDateTime READ liveUtcDateTime NOTIFY liveClockChanged)
    Q_PROPERTY(QString liveLocalDateTime READ liveLocalDateTime NOTIFY liveClockChanged)
    Q_PROPERTY(QString liveTimezone READ liveTimezone NOTIFY liveClockChanged)

    // Timestamp → DateTime
    Q_PROPERTY(QString timestampInput READ getTimestampInput WRITE setTimestampInput NOTIFY timestampInputChanged)
    Q_PROPERTY(bool isMilliseconds READ isMilliseconds WRITE setIsMilliseconds NOTIFY isMillisecondsChanged)
    Q_PROPERTY(QString tsResultUtc READ tsResultUtc NOTIFY tsResultChanged)
    Q_PROPERTY(QString tsResultLocal READ tsResultLocal NOTIFY tsResultChanged)
    Q_PROPERTY(QString tsResultRelative READ tsResultRelative NOTIFY tsResultChanged)

    // DateTime → Timestamp
    Q_PROPERTY(QString dateTimeInput READ getDateTimeInput WRITE setDateTimeInput NOTIFY dateTimeInputChanged)
    Q_PROPERTY(QStringList timezoneModel READ timezoneModel CONSTANT)
    Q_PROPERTY(int dtTimezoneIndex READ dtTimezoneIndex WRITE setDtTimezoneIndex NOTIFY dtTimezoneIndexChanged)
    Q_PROPERTY(QString dtResultSeconds READ dtResultSeconds NOTIFY dtResultChanged)
    Q_PROPERTY(QString dtResultMillis READ dtResultMillis NOTIFY dtResultChanged)

    Q_PROPERTY(QString errorMessage READ getErrorMessage NOTIFY errorMessageChanged)

public:
    explicit TimestampToolController(QObject* parent = nullptr);
    ~TimestampToolController() override;

    // Live clock
    QString liveTimestamp() const;
    QString liveUtcDateTime() const;
    QString liveLocalDateTime() const;
    QString liveTimezone() const;

    // Timestamp → DateTime
    QString getTimestampInput() const;
    void setTimestampInput(const QString& text);
    bool isMilliseconds() const;
    void setIsMilliseconds(bool ms);
    QString tsResultUtc() const;
    QString tsResultLocal() const;
    QString tsResultRelative() const;

    // DateTime → Timestamp
    QString getDateTimeInput() const;
    void setDateTimeInput(const QString& text);
    QStringList timezoneModel() const;
    int dtTimezoneIndex() const;
    void setDtTimezoneIndex(int index);
    QString dtResultSeconds() const;
    QString dtResultMillis() const;

    QString getErrorMessage() const;

    Q_INVOKABLE void timestampToDateTime();
    Q_INVOKABLE void dateTimeToTimestamp();
    Q_INVOKABLE void copyLiveTimestamp();
    Q_INVOKABLE void copyText(const QString& text);

protected:
    void init() override;

signals:
    void liveClockChanged();
    void timestampInputChanged();
    void isMillisecondsChanged();
    void tsResultChanged();
    void dateTimeInputChanged();
    void dtTimezoneIndexChanged();
    void dtResultChanged();
    void errorMessageChanged();

private:
    void updateLiveClock();
    void initTimezoneModel();
    void appendTimezone(const QString& label, const QTimeZone& tz);

    std::shared_ptr<commonHead::viewModels::IToolsViewModel> m_toolsViewModel;
    QTimer m_clockTimer;

    // Live clock
    QString m_liveTimestamp;
    QString m_liveUtcDateTime;
    QString m_liveLocalDateTime;
    QString m_liveTimezone;

    // Inputs
    QString m_timestampInput;
    QString m_dateTimeInput;
    bool m_isMilliseconds = false;
    QStringList m_timezoneModel;
    QList<QTimeZone> m_timezones;
    int m_dtTimezoneIndex = 0;

    // Timestamp → DateTime results
    QString m_tsResultUtc;
    QString m_tsResultLocal;
    QString m_tsResultRelative;

    // DateTime → Timestamp results
    QString m_dtResultSeconds;
    QString m_dtResultMillis;

    QString m_errorMessage;
};
