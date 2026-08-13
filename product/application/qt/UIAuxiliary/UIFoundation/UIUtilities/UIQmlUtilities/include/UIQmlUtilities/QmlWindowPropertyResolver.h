#pragma once

#include <concepts>

#include <QtCore/QObject>
#include <QtCore/QVariant>

#include <UIQmlUtilities/UIQmlUtilitiesExport.h>

class QQuickWindow;

namespace UIUtilities {

// Resolves a named property from a QML window or, for an Item-as-root view,
// from the QQuickView's root object. The returned object remains owned by the
// QML object tree.
class UIQmlUtilities_EXPORT QmlWindowPropertyResolver final
{
public:
    static QVariant resolve(QQuickWindow* window, const char* propertyName);

    template <typename ObjectT>
        requires std::derived_from<ObjectT, QObject>
    static ObjectT* resolveObjectAs(QQuickWindow* window, const char* propertyName)
    {
        const QVariant value = resolve(window, propertyName);
        if (!value.isValid())
        {
            return nullptr;
        }
        return qobject_cast<ObjectT*>(value.value<QObject*>());
    }
};

} // namespace UIUtilities
