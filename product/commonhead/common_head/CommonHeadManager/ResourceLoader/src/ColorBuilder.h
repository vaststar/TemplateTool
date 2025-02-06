#pragma once

#include <memory>
#include <vector>
#include <string>

namespace ucf::service::model
{
    enum class ThemeType;
}
namespace commonHead{

namespace model{
    class Color;
    enum class ColorItem;
}

class Colors;
class ColorBuilder final
{
public:
    std::vector<std::shared_ptr<Colors>> buildColors(ucf::service::model::ThemeType themeType);
private:
    std::vector<std::shared_ptr<Colors>> buildColorsFromVec(const std::vector<std::pair<model::ColorItem, std::vector<std::string>>>& colorVec);
    model::Color buildColorFromRGB(const std::string& rgbString);
    int hexCharToInt(char c);
    int hexPairToInt(const std::string& hexPair);
private:
    std::vector<std::pair<model::ColorItem, std::vector<std::string>>> generateDarkColors();
    std::vector<std::pair<model::ColorItem, std::vector<std::string>>> generateLightColors();
};
}