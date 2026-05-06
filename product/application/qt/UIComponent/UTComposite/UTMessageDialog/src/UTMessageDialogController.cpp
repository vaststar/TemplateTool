#include <UTMessageDialog/UTMessageDialogController.h>

#include <QtCore/QVariantMap>

#include "LoggerDefine.h"

UTMessageDialogController::UTMessageDialogController(QObject* parent)
    : QObject(parent)
{
}

namespace {

// Validates and clamps `isDefault` / `isCancel` to at most one each, in place.
void normalizeButtons(QList<UTMessageButton>& buttons)
{
    int defaultIdx = -1;
    int cancelIdx  = -1;
    for (int i = 0; i < buttons.size(); ++i)
    {
        if (buttons[i].isDefault)
        {
            if (defaultIdx == -1)
            {
                defaultIdx = i;
            }
            else
            {
                UICOM_LOG_WARN("UTMessageOptions: multiple isDefault=true; clearing extra at index " << i);
                Q_ASSERT_X(false, "UTMessageDialogController::setOptions", "multiple isDefault buttons");
                buttons[i].isDefault = false;
            }
        }
        if (buttons[i].isCancel)
        {
            if (cancelIdx == -1)
            {
                cancelIdx = i;
            }
            else
            {
                UICOM_LOG_WARN("UTMessageOptions: multiple isCancel=true; clearing extra at index " << i);
                Q_ASSERT_X(false, "UTMessageDialogController::setOptions", "multiple isCancel buttons");
                buttons[i].isCancel = false;
            }
        }
    }
}

QVariantList toButtonsModel(const QList<UTMessageButton>& buttons)
{
    QVariantList model;
    model.reserve(buttons.size());
    for (const auto& b : buttons)
    {
        QVariantMap m;
        m.insert(QStringLiteral("text"),      b.text);
        m.insert(QStringLiteral("tooltip"),   b.tooltip);
        m.insert(QStringLiteral("role"),      static_cast<int>(b.role));
        m.insert(QStringLiteral("isDefault"), b.isDefault);
        m.insert(QStringLiteral("isCancel"),  b.isCancel);
        m.insert(QStringLiteral("enabled"),   b.enabled);
        model.append(m);
    }
    return model;
}

} // namespace

void UTMessageDialogController::setOptions(const UTMessageOptions& options)
{
    if (mOptionsSet)
    {
        UICOM_LOG_WARN("UTMessageDialogController::setOptions called more than once; ignoring");
        return;
    }
    mOptionsSet = true;

    mOptions = options;
    normalizeButtons(mOptions.buttons);

    mButtonsModel = toButtonsModel(mOptions.buttons);

    mCancelIndex = -1;
    for (int i = 0; i < mOptions.buttons.size(); ++i)
    {
        if (mOptions.buttons.at(i).isCancel)
        {
            mCancelIndex = i;
            break;
        }
    }

    emit optionsChanged();
}

void UTMessageDialogController::accept(int index)
{
    if (mFired)
    {
        return;
    }
    mFired = true;

    UTMessageResult result;
    result.buttonIndex = index;
    if (index >= 0 && index < mOptions.buttons.size())
    {
        result.buttonText = mOptions.buttons.at(index).text;
    }
    emit closed(result);
}
