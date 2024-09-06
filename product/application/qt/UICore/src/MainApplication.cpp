#include <UICore/MainApplication.h>

MainApplication::MainApplication(int& argc, char** argv)
    : QApplication(argc, argv)
    , mApplicationEngine(new QQmlApplicationEngine(this))
{

}