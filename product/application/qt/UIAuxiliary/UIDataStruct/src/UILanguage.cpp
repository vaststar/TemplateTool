#include <UIDataStruct/UILanguage.h>
#include <QQmlEngine>

namespace UILanguage{
void registerMetaObject()
{
    qmlRegisterUncreatableMetaObject(
	        UILanguage::staticMetaObject,      // The meta-object of the namespace
	        "UILanguage",                      // The URI or module name
	        1, 0,                          // Version
	        "UILanguage",                      // The name used in QML
	        "Access to enums only"         // Error message for attempting to create an instance
	    );
}
}