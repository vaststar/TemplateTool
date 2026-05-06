#pragma once

#include <QtCore/QList>
#include <QtCore/QMetaType>
#include <QtCore/QString>

class QWindow;

enum class UTMessageIcon
{
    None,
    Info,
    Warning,
    Error,
    Question,
    Success,
};

enum class UTButtonRole
{
    Accept,        // Primary action (OK / Save / Yes)
    Reject,        // Cancel-style action (Cancel / No)
    Destructive,   // Dangerous action (Delete) - typically rendered emphasised
    Neutral,       // No strong semantics
};

// Strongly-typed description of one dialog button.
//
// `isDefault` / `isCancel` are validated at runtime by
// UTMessageDialogController::setOptions(): at most one of each per dialog.
// Extra flags are cleared (with assert).
struct UTMessageButton
{
    QString      text;
    QString      tooltip;
    UTButtonRole role      = UTButtonRole::Neutral;
    bool         isDefault = false;   // Triggered by Enter
    bool         isCancel  = false;   // Triggered by Esc / window close
    bool         enabled   = true;
};

struct UTMessageResult
{
    int          buttonIndex = -1;                       // -1: window closed externally with no cancel button
    QString      buttonText;
    UTButtonRole role        = UTButtonRole::Neutral;    // Role of the clicked button; meaningless when buttonIndex == -1
};

struct UTMessageOptions
{
    QString                title;
    QString                message;
    QString                detail;        // Optional secondary description
    UTMessageIcon          icon = UTMessageIcon::Info;
    QList<UTMessageButton> buttons;       // Must contain at least one entry
    QWindow*               parent = nullptr;
};

Q_DECLARE_METATYPE(UTMessageResult)
