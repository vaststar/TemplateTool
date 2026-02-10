#pragma once
#include <UICore/CoreApplication.h>
#include <UIAppCore/UIAppCoreExport.h>

namespace UIAppCore {

class UIAppCore_EXPORT UIApplication : public UICore::CoreApplication
{
    Q_OBJECT
public:
    UIApplication(int& argc, char** argv);
    ~UIApplication();
};

}
