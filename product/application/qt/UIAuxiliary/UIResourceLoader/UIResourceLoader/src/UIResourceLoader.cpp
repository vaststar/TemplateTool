#include <UIResourceLoader/UIResourceLoader.h>
#include <UIResourceColorLoader/UIResourceColorLoader.h>
#include <UIResourceStringLoader/UIResourceStringLoader.h>
#include <QQmlEngine>
namespace UIResource{
void UIResourceLoader::registerMetaObject()
{
    UIResourceColorLoader::registerMetaObject();
    UIResourceStringLoader::registerMetaObject();
}
};