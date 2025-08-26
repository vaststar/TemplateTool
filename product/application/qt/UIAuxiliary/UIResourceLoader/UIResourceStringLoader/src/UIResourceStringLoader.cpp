#include <UIResourceStringLoader/UIResourceStringLoader.h>

#include <QQmlEngine>

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>
#include <commonHead/ResourceLoader/IResourceLoader.h>
#include <commonHead/ResourceStringLoader/IResourceStringLoader.h>

#include "AppUIStringLoader.h"

namespace UIResource{
void UIResourceStringLoader::registerMetaObject()
{
    qmlRegisterUncreatableMetaObject(
	    UIStringToken::staticMetaObject,      // The meta-object of the namespace
	    "UIResourceLoader",                      // The URI or module name
	    1, 0,                          // Version
	    "UIStringToken",                      // The name used in QML
	    "Access to enums only"         // Error message for attempting to create an instance
	);
}
void UIResourceStringLoader::registerResourceStringLoader(commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
{
    if (auto chFramework = commonheadFramework.lock())
    {
        if (auto resourceLoader = chFramework->getResourceLoader())
        {
            resourceLoader->setLocalizedStringLoader(std::make_unique<AppUIStringLoader>());
        }
    }
}
}