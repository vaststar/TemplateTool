#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "winLogger.h"
#include "ICoreFramework.h"
#include "CoreFramework.h"
#include "ImageService.h"
#include "ThreadPoolUtil.h"
int main(int argc, char *argv[])
{
	std::string dirPath = "./app_log";
	std::string baseFileName_app = "app";
	auto configure_app = std::make_shared<LogLogSpace::LoggerFileConfigure>(MasterLogUtil::ALL_LOG_LEVEL, dirPath, baseFileName_app, 180, 20*1024, "APP");
	MasterLogUtil::InitLogger({configure_app});

    ThreadPoolUtil::initThreadPool(3);
    std::shared_ptr<ICoreFramework> a = std::make_shared<CoreFramework>();
    std::shared_ptr<IImageService> b = std::make_shared<ImageService>(std::weak_ptr(a));

    WIN_LOG_DEBUG("start app succeed:" + a->getName() + std::to_string(a->getServices().size()) + b->getServiceName());




    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/qt/qml/mainqml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}