#include <UIDataStruct/UIElementData.h>
#include <QQmlEngine>

namespace UIElementData{
void registerMetaObject()
{
    qmlRegisterUncreatableMetaObject(
		        UIElementData::staticMetaObject,      // The meta-object of the namespace
		        "UIElementData",                      // The URI or module name
		        1, 0,                          // Version
		        "UIElementData",                      // The name used in QML
		        "Access to enums only"         // Error message for attempting to create an instance
		    );
}
}