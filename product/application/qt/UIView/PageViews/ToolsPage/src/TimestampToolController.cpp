#include "PageViews/ToolsPage/include/TimestampToolController.h"
#include "LoggerDefine/LoggerDefine.h"

#include <commonHead/viewModels/ToolsViewModel/IToolsViewModel.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>
#include <QClipboard>
#include <QGuiApplication>

TimestampToolController::TimestampToolController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create TimestampToolController");
}

void TimestampToolController::init()
{
    UIVIEW_LOG_DEBUG("TimestampToolController::init");
    m_toolsViewModel = getAppContext()->getViewModelFactory()->createToolsViewModelInstance();
    m_toolsViewModel->initViewModel();
}

QString TimestampToolController::getTimestampInput() const
{
    return m_timestampInput;
}

void TimestampToolController::setTimestampInput(const QString& text)
{
    if (m_timestampInput != text) {
        m_timestampInput = text;
        emit timestampInputChanged();
    }
}

QString TimestampToolController::getDateTimeInput() const
{
    return m_dateTimeInput;
}

void TimestampToolController::setDateTimeInput(const QString& text)
{
    if (m_dateTimeInput != text) {
        m_dateTimeInput = text;
        emit dateTimeInputChanged();
    }
}

QString TimestampToolController::getResultText() const
{
    return m_resultText;
}

QString TimestampToolController::getErrorMessage() const
{
    return m_errorMessage;
}

bool TimestampToolController::isMilliseconds() const
{
    return m_isMilliseconds;
}

void TimestampToolController::setIsMilliseconds(bool ms)
{
    if (m_isMilliseconds != ms) {
        m_isMilliseconds = ms;
        emit isMillisecondsChanged();
    }
}

void TimestampToolController::timestampToDateTime()
{
    if (!m_toolsViewModel)
        return;

    bool ok = false;
    int64_t timestamp = m_timestampInput.toLongLong(&ok);
    
    if (!ok) {
        m_errorMessage = "无效的时间戳";
        m_resultText.clear();
        emit errorMessageChanged();
        emit resultTextChanged();
        return;
    }

    auto result = m_toolsViewModel->timestampToDateTime(timestamp, m_isMilliseconds);
    
    if (result.success) {
        m_resultText = QString::fromStdString(result.dateTimeStr) + 
                       " (" + QString::fromStdString(result.timezone) + ")";
        m_errorMessage.clear();
    } else {
        m_resultText.clear();
        m_errorMessage = QString::fromStdString(result.errorMessage);
    }
    
    emit resultTextChanged();
    emit errorMessageChanged();
}

void TimestampToolController::dateTimeToTimestamp()
{
    if (!m_toolsViewModel)
        return;

    auto result = m_toolsViewModel->dateTimeToTimestamp(m_dateTimeInput.toStdString());
    
    if (result.success) {
        m_resultText = QString::number(result.timestamp);
        m_errorMessage.clear();
    } else {
        m_resultText.clear();
        m_errorMessage = QString::fromStdString(result.errorMessage);
    }
    
    emit resultTextChanged();
    emit errorMessageChanged();
}

void TimestampToolController::getCurrentTimestamp()
{
    if (!m_toolsViewModel)
        return;

    auto result = m_toolsViewModel->getCurrentTimestamp();
    
    m_timestampInput = QString::number(result.timestamp);
    m_resultText = QString::fromStdString(result.dateTimeStr) + 
                   " (" + QString::fromStdString(result.timezone) + ")";
    m_errorMessage.clear();
    
    emit timestampInputChanged();
    emit resultTextChanged();
    emit errorMessageChanged();
}

void TimestampToolController::copyResult()
{
    QClipboard* clipboard = QGuiApplication::clipboard();
    if (clipboard) {
        clipboard->setText(m_resultText);
    }
}
