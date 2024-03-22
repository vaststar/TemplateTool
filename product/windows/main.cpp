#include <QGuiApplication>
#include <QQmlApplicationEngine>
//#include <QtWebEngineQuick>

#include <memory>
#include "winLogger.h"
#include "CoreFramework/ICoreFramework.h"
#include "CoreFramework/CoreFramework.h"
#include "ImageService/ImageService.h"
#include "ContactService/IContactService.h"

#include "ThreadPool/ThreadPoolUtil.h"
#include "CommonHeadFramework/ICommonHeadFramework.h"
#include "ContactListViewModel/IContactListViewModel.h"



class test :public IContactServiceCallback
{
    virtual void OnContactListAvailable() {
        WIN_LOG_DEBUG("receive OnContactListAvailable");
    }
};
int main(int argc, char *argv[])
{
	std::string dirPath = "./app_log";
	std::string baseFileName_app = "app";
	auto configure_app = std::make_shared<LogLogSpace::LoggerFileConfigure>(MasterLogUtil::ALL_LOG_LEVEL, dirPath, baseFileName_app, 180, 20*1024, "APP");
	MasterLogUtil::InitLogger({configure_app});

    ThreadPoolUtil::initThreadPool(3);
    std::shared_ptr<ICoreFramework> a = std::make_shared<CoreFramework>();
    std::shared_ptr<ICommonHeadFramework> commonHeadFramework = ICommonHeadFramework::CreateInstance();

    std::shared_ptr<IImageService> b = std::make_shared<ImageService>(std::weak_ptr(a));
    auto contactService = IContactService::CreateInstance(std::weak_ptr(a));

    auto testListen = std::make_shared<test>();
    contactService->registerCallback(testListen);
    contactService->fetchContactList();




    WIN_LOG_DEBUG("start app succeed:" + a->getName() + std::to_string(a->getServices().size()) + contactService->getServiceName());

    auto contactListViewModel = CommonHead::ViewModels::IContactListViewModel::CreateInstance(std::weak_ptr(commonHeadFramework));

    //QtWebEngineQuick::initialize();
    //QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

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