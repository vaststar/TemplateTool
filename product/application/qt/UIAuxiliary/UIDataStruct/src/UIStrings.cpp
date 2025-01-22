#include <UIDataStruct/UIStrings.h>
#include <QQmlEngine>
#include <QCoreApplication>
#include <QString>

namespace UIStrings{
void registerMetaObject()
{
    qmlRegisterUncreatableMetaObject(
		        UIStrings::staticMetaObject,      // The meta-object of the namespace
		        "UIStrings",                      // The URI or module name
		        1, 0,                          // Version
		        "UIStrings",                      // The name used in QML
		        "Access to enums only"         // Error message for attempting to create an instance
		    );
}
}