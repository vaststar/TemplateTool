#include <UIResourceLoader/UIResourceLoader.h>
#include <UIResourceColorLoader/UIResourceColorLoader.h>
#include <QQmlEngine>
namespace UIResouce{
void UIResourceLoader::registerMetaObject()
{
    UIResourceColorLoader::registerMetaObject();
}
};