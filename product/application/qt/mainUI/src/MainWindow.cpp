#include "MainWindow/MainWindow.h"

#include <QGuiApplication>
#include <QQuickView>
#include <QQmlApplicationEngine>

#include "CommonHeadFramework/ICommonHeadFramework.h"

#include "ClientGlobal/ClientGlobal.h"
#include "MainWindow/LoggerDefine.h"
#include "MainWindow/MainWindowController.h"



MainWindow::MainWindow(ICommonHeadFrameworkWPtr commonHeadFramework)
    : mCommonHeadFrameworkWPtr(commonHeadFramework)
{
    MAINUI_LOG_DEBUG("create MainWindow with CommonHeadFramework, address: " << commonHeadFramework.lock());
    ClientGlobal::getInstance()->setCommonHeadFramework(commonHeadFramework);
}

int MainWindow::runMainWindow(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    const QUrl url(QStringLiteral("qrc:/qt/qml/mainUI/qml/MainWindow.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app, [url, this](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
        {
            QCoreApplication::exit(-1);
        }
        // if (obj && url == objUrl)
        // {
        //     MainWindowController controller(mCommonHeadFrameworkWPtr);
        //     obj->setProperty("controller", QVariant::fromValue(&controller));
        //     MAINUI_LOG_DEBUG("set controller");
        // }
    }, Qt::QueuedConnection);
    engine.load(url);
    return app.exec();
}