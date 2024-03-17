#include <QGuiApplication>
#include <QQmlApplicationEngine>
//#include "cpp/Datas.h"
#include "LogExport.h"
#include "ICoreFramework.h"
#include "CoreFramework.h"

int main(int argc, char *argv[])
{
    #define TEST_DEFAULT_FILE_LOG_DEBUG(message) LOG_DEBUG("TEST_DEFAULT_FILE_LOG",message,MasterLogUtil::Default_File_Logger_Name)
#define TEST_APP_FILE_LOG_DEBUG(message) LOG_DEBUG("TEST_APP_FILE_LOG",message,"APP")
	std::string dirPath = "./app_log";
	std::string baseFileName_app = "app";
	auto configure_app = std::make_shared<LogLogSpace::LoggerFileConfigure>(MasterLogUtil::ALL_LOG_LEVEL, dirPath, baseFileName_app, 180, 20*1024, "APP");
	MasterLogUtil::InitLogger({configure_app});

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);
    // qmlRegisterType<Datas>("MyData",1,0,"Datas");

    CoreFramework a;
TEST_APP_FILE_LOG_DEBUG("start app succeed" + a.getName());
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