#include "PermissionGate.h"

namespace MiniAppRuntime {

void PermissionGate::setGranted(const QStringList& permissions)
{
    m_granted = QSet<QString>(permissions.cbegin(), permissions.cend());
}

bool PermissionGate::allows(const QStringList& required) const
{
    for (const QString& token : required)
    {
        if (!m_granted.contains(token))
        {
            return false;
        }
    }
    return true;
}

} // namespace MiniAppRuntime
