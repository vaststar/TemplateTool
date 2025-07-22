#pragma once

#include <vector>

namespace ucf::service{

namespace model{
enum class ThemeType;
}

class IClientInfoServiceCallback
{
public:
    virtual ~ IClientInfoServiceCallback() = default;
    virtual void onClientInfoReady()  = 0;
    virtual void onClientThemeChanged(ucf::service::model::ThemeType themeType) = 0;
};
}