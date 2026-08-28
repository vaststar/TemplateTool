#include "ToolsPage/textprocess/Base64ToolController.h"
#include "LoggerDefine.h"

#include <commonhead/viewmodels/Base64ToolViewModel/IBase64ToolViewModel.h>
#include <commonhead/viewmodels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>
#include <QClipboard>
#include <QGuiApplication>

Base64ToolController::Base64ToolController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("Base64ToolController constructed, address: " << this);
}

Base64ToolController::~Base64ToolController()
{
    UIVIEW_LOG_DEBUG("Base64ToolController destroying, address: " << this);
}

void Base64ToolController::init()
{
    UIVIEW_LOG_DEBUG("Base64ToolController::init");
    m_base64ViewModel = getViewModelFactory()->createBase64ToolViewModelInstance();
    m_base64ViewModel->initViewModel();
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
    if (!m_base64ViewModel)
        return;

    auto result = m_base64ViewModel->encode(m_inputText.toStdString(), m_urlSafe);

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
    if (!m_base64ViewModel)
        return;

    auto result = m_base64ViewModel->decode(m_inputText.toStdString());

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
