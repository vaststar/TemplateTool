#include <UIResourceLoader/UIResourceLoader.h>

#include <QQmlEngine>

#include <commonhead/CommonHeadFramework/ICommonHeadFramework.h>

#include <UIResourceColorLoader/UIResourceColorLoader.h>
#include <UIResourceStringLoader/UIResourceStringLoader.h>
#include <UIResourceFontLoader/UIResourceFontLoader.h>
#include <UIResourceAssetLoader/UIResourceAssetLoader.h>

#include "LoggerDefine.h"

namespace UIResource{
void UIResourceLoader::registerUIResourceLoader(commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
{
    UIResourceLoader_LOG_DEBUG("UIResourceLoader registration started");

    UIResourceColorLoader::registerUIResourceColorLoader();
    UIResourceStringLoader::registerUIResourceStringLoader(commonheadFramework);
    UIResourceFontLoader::registerUIResourceFontLoader();
    UIResourceAssetLoader::registerUIResourceAssetLoader();

    UIResourceLoader_LOG_DEBUG("UIResourceLoader registration finished");
}
};
