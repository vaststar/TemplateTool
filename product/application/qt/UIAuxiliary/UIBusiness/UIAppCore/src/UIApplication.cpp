#include <UIAppCore/UIApplication.h>

namespace UIAppCore {

UIApplication::UIApplication(int& argc, char** argv)
    : UICore::CoreApplication(argc, argv)
{
}

UIApplication::~UIApplication()
{
}

}
