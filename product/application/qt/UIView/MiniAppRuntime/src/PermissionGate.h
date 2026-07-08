#pragma once

#include <QSet>
#include <QString>
#include <QStringList>

namespace MiniAppRuntime {

// Holds the capability tokens granted to a mini-app and checks whether a
// required set is fully covered.
class PermissionGate
{
public:
    void setGranted(const QStringList& permissions);

    // True if every required token was granted; empty requirement is always allowed.
    [[nodiscard]] bool allows(const QStringList& required) const;

private:
    QSet<QString> m_granted;
};

} // namespace MiniAppRuntime
