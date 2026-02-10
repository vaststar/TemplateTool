#include <UIResourceLoader/UIResourceLoader.h>

#include <QQmlEngine>

#include <commonHead/CommonHeadFramework/ICommonHeadFramework.h>

#include <UIResourceColorLoader/UIResourceColorLoader.h>
#include <UIResourceStringLoader/UIResourceStringLoader.h>
#include <UIResourceFontLoader/UIResourceFontLoader.h>
#include <UIResourceAssetLoader/UIResourceAssetLoader.h>

namespace UIResource{
void UIResourceLoader::registerUIResourceLoader(commonHead::ICommonHeadFrameworkWPtr commonheadFramework)
{
    UIResourceColorLoader::registerUIResourceColorLoader();
    UIResourceStringLoader::registerUIResourceStringLoader(commonheadFramework);
    UIResourceFontLoader::registerUIResourceFontLoader();
    UIResourceAssetLoader::registerUIResourceAssetLoader();
}
};