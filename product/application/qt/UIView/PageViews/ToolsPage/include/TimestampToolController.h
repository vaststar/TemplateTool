#pragma once

#include <QObject>
#include <QtQml>
#include "UIViewBase/include/UIViewController.h"

namespace commonHead::viewModels {
    class IToolsViewModel;
}

class TimestampToolController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString timestampInput READ getTimestampInput WRITE setTimestampInput NOTIFY timestampInputChanged)
    Q_PROPERTY(QString dateTimeInput READ getDateTimeInput WRITE setDateTimeInput NOTIFY dateTimeInputChanged)
    Q_PROPERTY(QString resultText READ getResultText NOTIFY resultTextChanged)
    Q_PROPERTY(QString errorMessage READ getErrorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(bool isMilliseconds READ isMilliseconds WRITE setIsMilliseconds NOTIFY isMillisecondsChanged)

public:
    explicit TimestampToolController(QObject* parent = nullptr);

    QString getTimestampInput() const;
    void setTimestampInput(const QString& text);
    QString getDateTimeInput() const;
    void setDateTimeInput(const QString& text);
    QString getResultText() const;
    QString getErrorMessage() const;
    bool isMilliseconds() const;
    void setIsMilliseconds(bool ms);

    Q_INVOKABLE void timestampToDateTime();
    Q_INVOKABLE void dateTimeToTimestamp();
    Q_INVOKABLE void getCurrentTimestamp();
    Q_INVOKABLE void copyResult();

protected:
    void init() override;

signals:
    void timestampInputChanged();
    void dateTimeInputChanged();
    void resultTextChanged();
    void errorMessageChanged();
    void isMillisecondsChanged();

private:
    std::shared_ptr<commonHead::viewModels::IToolsViewModel> m_toolsViewModel;
    QString m_timestampInput;
    QString m_dateTimeInput;
    QString m_resultText;
    QString m_errorMessage;
    bool m_isMilliseconds = true;
};
