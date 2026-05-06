#include <UTMessageDialog/UTMessageDialogController.h>

#include <QtCore/QVariantMap>

#include <UIAssetToken.h>

#include "LoggerDefine.h"

UTMessageDialogController::UTMessageDialogController(QObject* parent)
    : QObject(parent)
{
}

void UTMessageDialogController::normalizeButtons(QList<UTMessageButton>& buttons)
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

QVariantList UTMessageDialogController::toButtonsModel(const QList<UTMessageButton>& buttons)
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

void UTMessageDialogController::setOptions(const UTMessageOptions& options)
{
    mOptions = options;
    normalizeButtons(mOptions.buttons);
    mFired = false;

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

int UTMessageDialogController::iconAssetToken() const
{
    switch (mOptions.icon)
    {
    case UTMessageIcon::Info:     return static_cast<int>(UIAssetImageToken::AssetImageToken::Msg_Info);
    case UTMessageIcon::Warning:  return static_cast<int>(UIAssetImageToken::AssetImageToken::Msg_Warning);
    case UTMessageIcon::Error:    return static_cast<int>(UIAssetImageToken::AssetImageToken::Msg_Error);
    case UTMessageIcon::Question: return static_cast<int>(UIAssetImageToken::AssetImageToken::Msg_Question);
    case UTMessageIcon::Success:  return static_cast<int>(UIAssetImageToken::AssetImageToken::Msg_Success);
    case UTMessageIcon::None:     break;
    }
    return 0;
}

QString UTMessageDialogController::iconColor() const
{
    switch (mOptions.icon)
    {
    case UTMessageIcon::Info:     return QStringLiteral("#2196F3");
    case UTMessageIcon::Warning:  return QStringLiteral("#FB8C00");
    case UTMessageIcon::Error:    return QStringLiteral("#E53935");
    case UTMessageIcon::Question: return QStringLiteral("#7E57C2");
    case UTMessageIcon::Success:  return QStringLiteral("#43A047");
    case UTMessageIcon::None:     break;
    }
    return {};
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
        const auto& button = mOptions.buttons.at(index);
        result.buttonText = button.text;
        result.role       = button.role;
    }
    emit closed(result);
}
