#include "PageViews/ToolsPage/textprocess/include/JsonToolController.h"
#include "PageViews/ToolsPage/textprocess/include/JsonTreeItemModel.h"
#include "UIViewCommon/LoggerDefine/LoggerDefine.h"

#include <commonHead/viewModels/ToolsViewModel/IToolsViewModel.h>
#include <commonHead/viewModels/JsonTreeViewModel/IJsonTreeViewModel.h>
#include <commonHead/viewModels/JsonTreeViewModel/IJsonTreeModel.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>
#include <QClipboard>
#include <QGuiApplication>

JsonToolController::JsonToolController(QObject* parent)
    : UIViewController(parent)
    , m_treeModel(new JsonTreeItemModel(this))
{
    UIVIEW_LOG_DEBUG("create JsonToolController");
}

void JsonToolController::init()
{
    UIVIEW_LOG_DEBUG("JsonToolController::init");
    m_toolsViewModel = getAppContext()->getViewModelFactory()->createToolsViewModelInstance();
    m_toolsViewModel->initViewModel();

    m_jsonTreeViewModel = getAppContext()->getViewModelFactory()->createJsonTreeViewModelInstance();
    m_jsonTreeViewModel->initViewModel();
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

    if (m_treeViewMode) {
        m_treeViewMode = false;
        emit treeViewModeChanged();
    }

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

    if (m_treeViewMode) {
        m_treeViewMode = false;
        emit treeViewModeChanged();
    }

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

    if (m_treeViewMode) {
        m_treeViewMode = false;
        emit treeViewModeChanged();
    }

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

    m_currentTree.reset();
    m_treeModel->clearTree();

    if (m_treeViewMode) {
        m_treeViewMode = false;
        emit treeViewModeChanged();
    }

    emit inputTextChanged();
    emit outputTextChanged();
    emit errorMessageChanged();
}

JsonTreeItemModel* JsonToolController::getTreeModel() const
{
    return m_treeModel;
}

bool JsonToolController::isTreeViewMode() const
{
    return m_treeViewMode;
}

void JsonToolController::parseTree()
{
    if (!m_jsonTreeViewModel)
        return;

    auto tree = m_jsonTreeViewModel->parseJsonTree(m_inputText.toStdString());
    if (!tree) {
        m_errorMessage = QString::fromStdString(m_jsonTreeViewModel->getLastError());
        m_currentTree.reset();
        m_treeModel->clearTree();
        m_treeViewMode = false;
    } else {
        m_errorMessage.clear();
        m_currentTree = tree;
        m_treeModel->setTree(tree);
        m_treeViewMode = true;
    }

    emit errorMessageChanged();
    emit treeViewModeChanged();
}

void JsonToolController::copyNodeValue(int row, int depth)
{
    Q_UNUSED(row)
    Q_UNUSED(depth)
    // Copy is handled via QML side using model data
}

QString JsonToolController::getNodeJsonPath(int row, int depth)
{
    Q_UNUSED(row)
    Q_UNUSED(depth)
    // JsonPath lookup is delegated to ViewModel via tree pointer
    // The QML side will pass the actual node pointer through the model index
    return {};
}

void JsonToolController::copySubTree(int row, int depth)
{
    Q_UNUSED(row)
    Q_UNUSED(depth)
    // SubTree copy is handled through ViewModel
}
