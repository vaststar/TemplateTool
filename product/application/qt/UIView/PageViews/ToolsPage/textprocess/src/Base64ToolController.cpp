#include "PageViews/ToolsPage/textprocess/include/Base64ToolController.h"
#include "LoggerDefine/LoggerDefine.h"

#include <commonHead/viewModels/ToolsViewModel/IToolsViewModel.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>
#include <QClipboard>
#include <QGuiApplication>

Base64ToolController::Base64ToolController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create Base64ToolController");
}

void Base64ToolController::init()
{
    UIVIEW_LOG_DEBUG("Base64ToolController::init");
    m_toolsViewModel = getAppContext()->getViewModelFactory()->createToolsViewModelInstance();
    m_toolsViewModel->initViewModel();
}

QString Base64ToolController::getInputText() const
{
    return m_inputText;
}

void Base64ToolController::setInputText(const QString& text)
{
    if (m_inputText != text) {
        m_inputText = text;
        emit inputTextChanged();
    }
}

QString Base64ToolController::getOutputText() const
{
    return m_outputText;
}

QString Base64ToolController::getErrorMessage() const
{
    return m_errorMessage;
}

bool Base64ToolController::isUrlSafe() const
{
    return m_urlSafe;
}

void Base64ToolController::setUrlSafe(bool urlSafe)
{
    if (m_urlSafe != urlSafe) {
        m_urlSafe = urlSafe;
        emit urlSafeChanged();
    }
}

void Base64ToolController::encode()
{
    if (!m_toolsViewModel)
        return;

    auto result = m_toolsViewModel->base64Encode(m_inputText.toStdString(), m_urlSafe);

    if (result.success) {
        m_outputText = QString::fromStdString(result.data);
        m_errorMessage.clear();
    } else {
        m_outputText.clear();
        m_errorMessage = QString::fromStdString(result.errorMessage);
    }

    emit outputTextChanged();
    emit errorMessageChanged();
}

void Base64ToolController::decode()
{
    if (!m_toolsViewModel)
        return;

    auto result = m_toolsViewModel->base64Decode(m_inputText.toStdString());

    if (result.success) {
        m_outputText = QString::fromStdString(result.data);
        m_errorMessage.clear();
    } else {
        m_outputText.clear();
        m_errorMessage = QString::fromStdString(result.errorMessage);
    }

    emit outputTextChanged();
    emit errorMessageChanged();
}

void Base64ToolController::swapInputOutput()
{
    QString temp = m_inputText;
    m_inputText = m_outputText;
    m_outputText = temp;

    emit inputTextChanged();
    emit outputTextChanged();
}

void Base64ToolController::copyOutput()
{
    QClipboard* clipboard = QGuiApplication::clipboard();
    if (clipboard) {
        clipboard->setText(m_outputText);
    }
}

void Base64ToolController::clearAll()
{
    m_inputText.clear();
    m_outputText.clear();
    m_errorMessage.clear();

    emit inputTextChanged();
    emit outputTextChanged();
    emit errorMessageChanged();
}
