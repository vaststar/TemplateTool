#pragma once

#include <memory>
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/ResourceColorLoader/model/Color.h>
#include <ColorToken.h>

namespace commonHead{
class COMMONHEAD_EXPORT IResourceColorLoader
{
public:
    IResourceColorLoader() = default;
    IResourceColorLoader(const IResourceColorLoader&) = delete;
    IResourceColorLoader(IResourceColorLoader&&) = delete;
    IResourceColorLoader& operator=(const IResourceColorLoader&) = delete;
    IResourceColorLoader& operator=(IResourceColorLoader&&) = delete;
    virtual ~IResourceColorLoader() = default;
public:

    virtual model::Color getColor(model::ColorToken token, model::ColorState state, model::ColorThemeType theme) const = 0;

    static std::unique_ptr<IResourceColorLoader> createInstance();
};
} // namespace commonHead