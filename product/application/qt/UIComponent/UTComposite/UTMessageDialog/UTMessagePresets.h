#pragma once

#include <UTMessageDialog/UTMessageButtonPresets.h>
#include <UTMessageDialog/UTMessageOptions.h>

// Convenience factories for the most common UTMessageOptions combinations.
//
// All factories build a fully populated options struct. Callers may tweak
// the result before handing it to UIViewHelper::showMessageAsync (e.g. add a
// third button, override the parent window).
namespace UTMessagePresets {

// Single-button [OK] notification.
inline UTMessageOptions info(QString title, QString message,
                             QString detail = {}, QWindow* parent = nullptr)
{
    return { std::move(title), std::move(message), std::move(detail),
             UTMessageIcon::Info,
             { UTMessageButtonPresets::ok() }, parent };
}

inline UTMessageOptions warning(QString title, QString message,
                                QString detail = {}, QWindow* parent = nullptr)
{
    return { std::move(title), std::move(message), std::move(detail),
             UTMessageIcon::Warning,
             { UTMessageButtonPresets::ok() }, parent };
}

inline UTMessageOptions error(QString title, QString message,
                              QString detail = {}, QWindow* parent = nullptr)
{
    return { std::move(title), std::move(message), std::move(detail),
             UTMessageIcon::Error,
             { UTMessageButtonPresets::ok() }, parent };
}

inline UTMessageOptions success(QString title, QString message,
                                QString detail = {}, QWindow* parent = nullptr)
{
    return { std::move(title), std::move(message), std::move(detail),
             UTMessageIcon::Success,
             { UTMessageButtonPresets::ok() }, parent };
}

// Two-button [Yes][No] confirmation. Default = Yes, Esc = No.
inline UTMessageOptions confirm(QString title, QString message,
                                QString detail = {}, QWindow* parent = nullptr)
{
    return { std::move(title), std::move(message), std::move(detail),
             UTMessageIcon::Question,
             { UTMessageButtonPresets::yes(), UTMessageButtonPresets::no() }, parent };
}

// Two-button [OK][Cancel]. Default = OK, Esc = Cancel.
inline UTMessageOptions okCancel(QString title, QString message,
                                 QString detail = {}, QWindow* parent = nullptr)
{
    return { std::move(title), std::move(message), std::move(detail),
             UTMessageIcon::Info,
             { UTMessageButtonPresets::ok(), UTMessageButtonPresets::cancel() }, parent };
}

// Destructive confirmation with a Warning icon.
// Layout: [Cancel] [<destructiveText>]. Esc = Cancel. The destructive button
// is intentionally NOT the default — users must opt in deliberately.
//
// Example: destructiveConfirm(tr("Delete file?"), tr("..."), tr("Delete"));
inline UTMessageOptions destructiveConfirm(QString title, QString message,
                                           QString destructiveText,
                                           QString detail = {}, QWindow* parent = nullptr)
{
    return { std::move(title), std::move(message), std::move(detail),
             UTMessageIcon::Warning,
             { UTMessageButtonPresets::cancel(),
               UTMessageButtonPresets::destructive(std::move(destructiveText)) },
             parent };
}

} // namespace UTMessagePresets
