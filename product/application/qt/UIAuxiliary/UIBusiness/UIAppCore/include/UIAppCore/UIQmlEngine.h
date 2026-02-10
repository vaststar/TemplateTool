#pragma once
#include <UICore/CoreQmlEngine.h>
#include <UIAppCore/UIAppCoreExport.h>

namespace UIAppCore {

class UIAppCore_EXPORT UIQmlEngine : public UICore::CoreQmlEngine
{
    Q_OBJECT
public:
    UIQmlEngine(QObject* parent = nullptr);
    ~UIQmlEngine();
};

}
