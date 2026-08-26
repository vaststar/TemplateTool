#include <catch2/catch_session.hpp>

#include <QByteArray>

#include <UIAppCore/UIApplication.h>

int main(int argc, char* argv[])
{
    qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));

    UIAppCore::UIApplication application(argc, argv);
    return Catch::Session().run(argc, argv);
}
