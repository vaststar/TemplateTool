#pragma once

#include "UIViewHelper/UIMessageOptions.h"

namespace UIView {

// Convenience factories for the most common dialog buttons.
// Pulled out of UIMessageOptions.h so translation units that only need the
// data types don't pull in these inline definitions.
namespace UIMessageButtonPresets {

inline UIMessageButton ok()
{
    return { QStringLiteral("OK"), {}, UIButtonRole::Accept, true, true, true };
}

inline UIMessageButton cancel()
{
    return { QStringLiteral("Cancel"), {}, UIButtonRole::Reject, false, true, true };
}

inline UIMessageButton yes()
{
    return { QStringLiteral("Yes"), {}, UIButtonRole::Accept, true, false, true };
}

inline UIMessageButton no()
{
    return { QStringLiteral("No"), {}, UIButtonRole::Reject, false, true, true };
}

inline UIMessageButton destructive(QString text)
{
    return { std::move(text), {}, UIButtonRole::Destructive, false, false, true };
}

} // namespace UIMessageButtonPresets
} // namespace UIView
