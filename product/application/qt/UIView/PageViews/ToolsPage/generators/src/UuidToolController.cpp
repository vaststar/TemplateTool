#include "PageViews/ToolsPage/generators/include/UuidToolController.h"
#include "LoggerDefine/LoggerDefine.h"

#include <commonHead/viewModels/ToolsViewModel/IToolsViewModel.h>
#include <commonHead/viewModels/ViewModelFactory/IViewModelFactory.h>
#include <AppContext/AppContext.h>
#include <QClipboard>
#include <QGuiApplication>

UuidToolController::UuidToolController(QObject* parent)
    : UIViewController(parent)
{
    UIVIEW_LOG_DEBUG("create UuidToolController");
}

void UuidToolController::init()
{
    UIVIEW_LOG_DEBUG("UuidToolController::init");
    m_toolsViewModel = getAppContext()->getViewModelFactory()->createToolsViewModelInstance();
    m_toolsViewModel->initViewModel();
}

QString UuidToolController::getGeneratedUuid() const
{
    return m_generatedUuid;
}

QString UuidToolController::getValidateInput() const
{
    return m_validateInput;
}

void UuidToolController::setValidateInput(const QString& text)
{
    if (m_validateInput != text) {
        m_validateInput = text;
        emit validateInputChanged();
    }
}

QString UuidToolController::getValidateResult() const
{
    return m_validateResult;
}

QStringList UuidToolController::getUuidHistory() const
{
    return m_uuidHistory;
}

void UuidToolController::generate()
{
    if (!m_toolsViewModel)
        return;

    m_generatedUuid = QString::fromStdString(m_toolsViewModel->generateUuid());
    m_uuidHistory.prepend(m_generatedUuid);

    // Limit history to 20 items
    while (m_uuidHistory.size() > 20) {
        m_uuidHistory.removeLast();
    }

    emit generatedUuidChanged();
    emit uuidHistoryChanged();
}

void UuidToolController::generateMultiple(int count)
{
    if (!m_toolsViewModel || count <= 0)
        return;

    for (int i = 0; i < count; ++i) {
        QString uuid = QString::fromStdString(m_toolsViewModel->generateUuid());
        m_uuidHistory.prepend(uuid);

        if (i == 0) {
            m_generatedUuid = uuid;
        }
    }

    // Limit history to 20 items
    while (m_uuidHistory.size() > 20) {
        m_uuidHistory.removeLast();
    }

    emit generatedUuidChanged();
    emit uuidHistoryChanged();
}

void UuidToolController::validate()
{
    if (!m_toolsViewModel)
        return;

    bool isValid = m_toolsViewModel->isValidUuid(m_validateInput.toStdString());
    m_validateResult = isValid ? tr("✓ Valid UUID") : tr("✗ Invalid UUID");

    emit validateResultChanged();
}

void UuidToolController::copyUuid()
{
    QClipboard* clipboard = QGuiApplication::clipboard();
    if (clipboard) {
        clipboard->setText(m_generatedUuid);
    }
}

void UuidToolController::copyHistory()
{
    QClipboard* clipboard = QGuiApplication::clipboard();
    if (clipboard) {
        clipboard->setText(m_uuidHistory.join("\n"));
    }
}

void UuidToolController::clearHistory()
{
    m_uuidHistory.clear();
    emit uuidHistoryChanged();
}
