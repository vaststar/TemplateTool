#include "PageViews/ToolsPage/include/JsonToolController.h"
#include "LoggerDefine/LoggerDefine.h"

#include <commonHead/viewModels/ToolsViewModel/IToolsViewModel.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>
#include <QClipboard>
#include <QGuiApplication>

JsonToolController::JsonToolController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create JsonToolController");
}

void JsonToolController::init()
{
    UIVIEW_LOG_DEBUG("JsonToolController::init");
    m_toolsViewModel = getAppContext()->getViewModelFactory()->createToolsViewModelInstance();
    m_toolsViewModel->initViewModel();
}

QString JsonToolController::getInputText() const
{
    return m_inputText;
}

void JsonToolController::setInputText(const QString& text)
{
    if (m_inputText != text) {
        m_inputText = text;
        emit inputTextChanged();
    }
}

QString JsonToolController::getOutputText() const
{
    return m_outputText;
}

QString JsonToolController::getErrorMessage() const
{
    return m_errorMessage;
}

int JsonToolController::getIndentSize() const
{
    return m_indentSize;
}

void JsonToolController::setIndentSize(int indent)
{
    if (m_indentSize != indent) {
        m_indentSize = indent;
        emit indentSizeChanged();
    }
}

void JsonToolController::format()
{
    if (!m_toolsViewModel)
        return;

    auto result = m_toolsViewModel->jsonFormat(m_inputText.toStdString(), m_indentSize);
    
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

void JsonToolController::minify()
{
    if (!m_toolsViewModel)
        return;

    auto result = m_toolsViewModel->jsonMinify(m_inputText.toStdString());
    
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

void JsonToolController::validate()
{
    if (!m_toolsViewModel)
        return;

    auto result = m_toolsViewModel->jsonValidate(m_inputText.toStdString());
    
    m_outputText = QString::fromStdString(result.data);
    m_errorMessage = result.success ? QString() : QString::fromStdString(result.errorMessage);
    
    emit outputTextChanged();
    emit errorMessageChanged();
}

void JsonToolController::copyOutput()
{
    QClipboard* clipboard = QGuiApplication::clipboard();
    if (clipboard) {
        clipboard->setText(m_outputText);
    }
}

void JsonToolController::clearAll()
{
    m_inputText.clear();
    m_outputText.clear();
    m_errorMessage.clear();
    
    emit inputTextChanged();
    emit outputTextChanged();
    emit errorMessageChanged();
}
