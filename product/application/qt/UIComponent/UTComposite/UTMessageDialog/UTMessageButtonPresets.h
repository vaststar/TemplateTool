#pragma once

#include <UTMessageDialog/UTMessageOptions.h>
#include <QtCore/QCoreApplication>

// Convenience factories for the most common dialog buttons.
namespace UTMessageButtonPresets {

// Sole acknowledgement button: Enter, Esc, and window close all dismiss the
// dialog through the same OK result.
inline UTMessageButton acknowledge()
{
    return { QCoreApplication::translate("UTMessageButtonPresets", "OK"), {}, UTButtonRole::Accept, true, true, true };
}

// OK action in a multi-button dialog. Esc/window close must be handled by a
// separate Cancel-style button.
inline UTMessageButton ok()
{
    return { QCoreApplication::translate("UTMessageButtonPresets", "OK"), {}, UTButtonRole::Accept, true, false, true };
}

inline UTMessageButton cancel()
{
    return { QCoreApplication::translate("UTMessageButtonPresets", "Cancel"), {}, UTButtonRole::Reject, false, true, true };
}

inline UTMessageButton yes()
{
    return { QCoreApplication::translate("UTMessageButtonPresets", "Yes"), {}, UTButtonRole::Accept, true, false, true };
}

inline UTMessageButton no()
{
    return { QCoreApplication::translate("UTMessageButtonPresets", "No"), {}, UTButtonRole::Reject, false, true, true };
}

inline UTMessageButton destructive(QString text)
{
    return { std::move(text), {}, UTButtonRole::Destructive, false, false, true };
}

} // namespace UTMessageButtonPresets
