#pragma once

#include <vector>
#include <ucf/Services/ServiceExportMacro/ServiceExport.h>

namespace ucf::service{

namespace model{
enum class ThemeType;
}

class SERVICE_EXPORT IClientInfoServiceCallback
{
public:
    virtual ~ IClientInfoServiceCallback() = default;
    virtual void onClientInfoReady()  = 0;
    virtual void onClientThemeChanged(ucf::service::model::ThemeType themeType) = 0;
};
}