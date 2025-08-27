#include <UIResourceStringLoader/UIResourceStringLoader.h>

#include <QQmlEngine>

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ResourceLoader/IResourceLoader.h>
#include <commonHead/ResourceStringLoader/IResourceStringLoader.h>

#include "AppUIStringLoader.h"
#include "LoggerDefine.h"

namespace UIResource{
void UIResourceStringLoader::registerUIResourceStringLoader(commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
{
    UIResourceStringLoader_LOG_DEBUG("");
    qmlRegisterUncreatableMetaObject(
	    UIStringToken::staticMetaObject,      // The meta-object of the namespace
	    "UIResourceLoader",                      // The URI or module name
	    1, 0,                          // Version
	    "UIStringToken",                      // The name used in QML
	    "Access to enums only"         // Error message for attempting to create an instance
	);

    if (auto chFramework = commonheadFramework.lock())
    {
        if (auto resourceLoader = chFramework->getResourceLoader())
        {
            resourceLoader->setLocalizedStringLoader(std::make_unique<AppUIStringLoader>());
        }
    }
}
}