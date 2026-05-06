#pragma once

#include <UTMessageDialog/UTMessageOptions.h>

// Convenience factories for the most common dialog buttons.
namespace UTMessageButtonPresets {

inline UTMessageButton ok()
{
    return { QStringLiteral("OK"), {}, UTButtonRole::Accept, true, true, true };
}

inline UTMessageButton cancel()
{
    return { QStringLiteral("Cancel"), {}, UTButtonRole::Reject, false, true, true };
}

inline UTMessageButton yes()
{
    return { QStringLiteral("Yes"), {}, UTButtonRole::Accept, true, false, true };
}

inline UTMessageButton no()
{
    return { QStringLiteral("No"), {}, UTButtonRole::Reject, false, true, true };
}

inline UTMessageButton destructive(QString text)
{
    return { std::move(text), {}, UTButtonRole::Destructive, false, false, true };
}

} // namespace UTMessageButtonPresets
