#include <UIResourceStringLoader/UIResourceStringLoader.h>

#include <QQmlEngine>

#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonhead/ResourceLoader/IResourceLoader.h>
#include <commonhead/ResourceStringLoader/IResourceStringLoader.h>

#include "AppUIStringLoader.h"
#include "LoggerDefine.h"

namespace UIResource{
void UIResourceStringLoader::registerUIResourceStringLoader(commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
{
    UIResourceStringLoader_LOG_DEBUG("UIResourceStringLoader registration started");

    qmlRegisterUncreatableMetaObject(
	    UILocalizedStringToken::staticMetaObject,      // The meta-object of the namespace
	    "UIResourceLoader",                            // The URI or module name
	    1, 0,                                          // Version
	    "UILocalizedStringToken",                      // The name used in QML
	    "Access to enums only"                         // Error message for attempting to create an instance
	);

    // qmlRegisterUncreatableMetaObject(
	//     UILocalizedStringWithParamToken::staticMetaObject,      // The meta-object of the namespace
	//     "UIResourceLoader",                                     // The URI or module name
	//     1, 0,                                                   // Version
	//     "UILocalizedStringWithParamToken",                      // The name used in QML
	//     "Access to enums only"                                  // Error message for attempting to create an instance
	// );
    // qmlRegisterUncreatableMetaObject(
	//     UINonLocalizedStringToken::staticMetaObject,      // The meta-object of the namespace
	//     "UIResourceLoader",                               // The URI or module name
	//     1, 0,                                             // Version
	//     "UINonLocalizedStringToken",                      // The name used in QML
	//     "Access to enums only"                            // Error message for attempting to create an instance
	// );

    if (auto chFramework = commonheadFramework.lock())
    {
        if (auto resourceLoader = chFramework->getResourceLoader())
        {
            resourceLoader->setLocalizedStringLoader(std::make_unique<AppUIStringLoader>());
        }
    }

    UIResourceStringLoader_LOG_DEBUG("UIResourceStringLoader registration finished");
}
}
