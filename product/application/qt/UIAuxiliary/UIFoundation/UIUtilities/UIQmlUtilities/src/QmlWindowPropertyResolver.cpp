#include <UIQmlUtilities/QmlWindowPropertyResolver.h>

#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickView>
#include <QtQuick/QQuickWindow>

namespace UIUtilities {

QVariant QmlWindowPropertyResolver::resolve(QQuickWindow* window, const char* propertyName)
{
    if (!window || !propertyName || propertyName[0] == '\0')
    {
        return {};
    }

    QVariant value = window->property(propertyName);
    if (value.isValid())
    {
        return value;
    }

    // Item-as-root QML is hosted by a QQuickView. In that case, application
    // properties belong to the root Item rather than to the container window.
    if (auto* view = qobject_cast<QQuickView*>(window))
    {
        if (auto* root = view->rootObject())
        {
            return root->property(propertyName);
        }
    }

    return {};
}

} // namespace UIUtilities
