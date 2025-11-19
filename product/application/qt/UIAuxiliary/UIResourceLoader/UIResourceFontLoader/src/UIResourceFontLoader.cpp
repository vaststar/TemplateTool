#include <UIResourceFontLoader/UIResourceFontLoader.h>

#include <QQmlEngine>

#include <UIFontToken.h>

#include "LoggerDefine.h"

namespace UIResource{

void UIResourceFontLoader::registerUIResourceFontLoader()
{
    UIResourceFontLoader_LOG_DEBUG("");
    qmlRegisterUncreatableMetaObject(
	    UIFontToken::staticMetaObject,      // The meta-object of the namespace
	    "UIResourceLoader",                      // The URI or module name
	    1, 0,                          // Version
	    "UIFontToken",                      // The name used in QML
	    "Access to enums only"         // Error message for attempting to create an instance
	);
}
}