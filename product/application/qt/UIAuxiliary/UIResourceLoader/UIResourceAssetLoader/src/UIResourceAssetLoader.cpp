#include <UIResourceAssetLoader/UIResourceAssetLoader.h>

#include <QQmlEngine>

#include <UIAssetToken.h>

#include "LoggerDefine.h"

namespace UIResource{

void UIResourceAssetLoader::registerUIResourceAssetLoader()
{
    UIResourceAssetLoader_LOG_DEBUG("");
    qmlRegisterUncreatableMetaObject(
	    UIAssetImageToken::staticMetaObject,      // The meta-object of the namespace
	    "UIResourceLoader",                      // The URI or module name
	    1, 0,                          // Version
	    "UIAssetImageToken",                      // The name used in QML
	    "Access to enums only"         // Error message for attempting to create an instance
	);
    qmlRegisterUncreatableMetaObject(
	    UIAssetVideoToken::staticMetaObject,      // The meta-object of the namespace
	    "UIResourceLoader",                      // The URI or module name
	    1, 0,                          // Version
	    "UIAssetVideoToken",                      // The name used in QML
	    "Access to enums only"         // Error message for attempting to create an instance
	);
}
}