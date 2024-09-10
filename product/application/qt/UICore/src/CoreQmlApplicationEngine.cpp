#include <UICore/CoreQmlApplicationEngine.h>

CoreQmlApplicationEngine::CoreQmlApplicationEngine(QObject* parent)
    : QQmlApplicationEngine(parent)
{

}

CoreQmlApplicationEngine::CoreQmlApplicationEngine(const QUrl& url, QObject* parent)
    : QQmlApplicationEngine(url, parent)
{

}