#include <UICore/CoreViewFactory.h>
#include <UICore/CoreQmlApplicationEngine.h>

#include <QFileInfo>
#include <iostream>
CoreViewFactory::CoreViewFactory(std::unique_ptr<CoreQmlApplicationEngine>&& qmlEngine)
    : mQmlEngine(std::move(qmlEngine))
{

}

CoreViewFactory::~CoreViewFactory() = default;
QPointer<QQuickView> CoreViewFactory::createQmlWindow(const QString& qmlResource, QWindow* parent, QObject* controller)
{
    if (qmlResource.isEmpty())
    {
        return nullptr;
    }

    auto view = new QQuickView(mQmlEngine.get(), parent);

    if (controller)
    {
        view->setInitialProperties({ {QStringLiteral("controller"), QVariant::fromValue(controller)} });
    }

    view->setSource(qmlResource);
    return view;
}


void CoreViewFactory::loadQmlWindow(const QString& qmlResource)
{
    mQmlEngine->load(qmlResource);
}   