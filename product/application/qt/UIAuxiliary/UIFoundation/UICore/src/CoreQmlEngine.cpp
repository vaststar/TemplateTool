#include <UICore/CoreQmlEngine.h>


namespace UICore{
CoreQmlEngine::CoreQmlEngine(QObject* parent)
    : QQmlApplicationEngine(parent)
{

}

CoreQmlEngine::CoreQmlEngine(const QUrl& url, QObject* parent)
    : QQmlApplicationEngine(url, parent)
{
}
}