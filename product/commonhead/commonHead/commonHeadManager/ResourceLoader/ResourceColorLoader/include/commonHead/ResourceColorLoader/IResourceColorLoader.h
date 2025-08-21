#pragma once

#include <memory>
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/ResourceColorLoader/model/Color.h>
#include <ColorToken.h>

namespace commonHead{
class COMMONHEAD_EXPORT IResourceColorLoader
{
public:
    virtual ~IResourceColorLoader() = default;

    virtual model::Color getColor(model::ColorToken token, model::ColorState state, model::ColorThemeType theme) const = 0;

    static std::shared_ptr<IResourceColorLoader> createInstance();
};
} // namespace commonHead