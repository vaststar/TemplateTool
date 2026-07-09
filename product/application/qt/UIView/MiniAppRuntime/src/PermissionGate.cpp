#include "PermissionGate.h"

namespace MiniAppRuntime {

void PermissionGate::setGranted(const QStringList& permissions)
{
    m_granted = QSet<QString>(permissions.cbegin(), permissions.cend());
}

QStringList PermissionGate::granted() const
{
    return QStringList(m_granted.cbegin(), m_granted.cend());
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
