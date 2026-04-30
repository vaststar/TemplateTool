#pragma once

#include <QtCore/QString>
#include <QtCore/QList>

class QWindow;

namespace UIView {

enum class UIMessageIcon
{
    None,
    Info,
    Warning,
    Error,
    Question,
    Success,
};

enum class UIButtonRole
{
    Accept,        // Primary action (OK / Save / Yes)
    Reject,        // Cancel-style action (Cancel / No)
    Destructive,   // Dangerous action (Delete) - typically rendered emphasised
    Neutral,       // No strong semantics
};

// Strongly-typed description of one dialog button.
//
// `isDefault` / `isCancel` are validated at runtime by UIViewHelper:
// at most one of each per dialog. Extra flags are cleared (with assert).
struct UIMessageButton
{
    QString      text;
    QString      tooltip;
    UIButtonRole role      = UIButtonRole::Neutral;
    bool         isDefault = false;   // Triggered by Enter
    bool         isCancel  = false;   // Triggered by Esc / window close
    bool         enabled   = true;
};

struct UIMessageResult
{
    int     buttonIndex = -1;   // -1: window closed externally with no cancel button
    QString buttonText;
};

struct UIMessageOptions
{
    QString                title;
    QString                message;
    QString                detail;        // Optional secondary description
    UIMessageIcon          icon = UIMessageIcon::Info;
    QList<UIMessageButton> buttons;       // Must contain at least one entry
    QWindow*               parent = nullptr;
};

} // namespace UIView
