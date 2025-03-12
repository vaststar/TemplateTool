#include "ColorBuilder.h"

#include <algorithm>
#include <ucf/Services/ClientInfoService/ClientInfoModel.h>

#include <commonHead/ResourceLoader/ResourceLoaderModel.h>
#include <commonHead/CommonHeadCommonFile/CommonHeadLogger.h>
#include "ColorSet.h"
#include "ColorConstant.h"

namespace commonHead{

std::vector<std::shared_ptr<Colors>> ColorBuilder::buildColors(ucf::service::model::ThemeType themeType)
{
    switch (themeType)
    {
    case ucf::service::model::ThemeType::SystemDefault:
        return buildColorsFromVec(generateDarkColors());
    case ucf::service::model::ThemeType::Dark:
        return buildColorsFromVec(generateDarkColors());
    case ucf::service::model::ThemeType::Light:
        return buildColorsFromVec(generateLightColors());
    default:
        break;
    }
    return {};
}

std::vector<std::pair<model::ColorItem, std::vector<std::string>>> ColorBuilder::generateDarkColors()
{
    //color enum, color set: normal, hovered, pressed, disabled, focused, checked
    return {
        {model::ColorItem::ColorItem_MAIN_WINDOW_BACKGROUND, {BlackThemeColorConstant::MainWindowBackground}},
        {model::ColorItem::ColorItem_Item_Focus_Border, {ItemColorConstant::FocusItemColor}},
        {model::ColorItem::ColorItem_Button_Primary_Text, {ColorConstant::Black100}},
        {model::ColorItem::ColorItem_Button_Primary_Background, {ColorConstant::Blue60, ColorConstant::Blue50, ColorConstant::Blue30, ColorConstant::Gray90, ColorConstant::Blue60, ColorConstant::Blue60}},
        {model::ColorItem::ColorItem_Button_Primary_Border, {ColorConstant::Blue100}}
    };
}

std::vector<std::pair<model::ColorItem, std::vector<std::string>>> ColorBuilder::generateLightColors()
{
    //color enum, color set: normal, hovered, pressed, disabled, focused, checked
    return {
        {model::ColorItem::ColorItem_MAIN_WINDOW_BACKGROUND, {WhiteThemeColorConstant::MainWindowBackground}},
        {model::ColorItem::ColorItem_Button_Primary_Text, {ColorConstant::Black100}},
        {model::ColorItem::ColorItem_Button_Primary_Background, {ColorConstant::Blue60, ColorConstant::Blue50, ColorConstant::Blue30, ColorConstant::Gray90, ColorConstant::Blue60, ColorConstant::Blue60}},
        {model::ColorItem::ColorItem_Button_Primary_Border, {ColorConstant::Blue100}}
    };
}

std::vector<std::shared_ptr<Colors>> ColorBuilder::buildColorsFromVec(const std::vector<std::pair<model::ColorItem, std::vector<std::string>>>& colorVec)
{
    std::vector<std::shared_ptr<Colors>> uiColors;
    std::transform(colorVec.cbegin(), colorVec.cend(), std::back_inserter(uiColors), [this](const auto& colorTuple){
        if (colorTuple.second.size() == 1)
        {
            return std::make_shared<Colors>(colorTuple.first, buildColorFromRGB(colorTuple.second[0]));  
        }
        else if (colorTuple.second.size() == 6)
        {
            return std::make_shared<Colors>(colorTuple.first, buildColorFromRGB(colorTuple.second[0]), buildColorFromRGB(colorTuple.second[1]), buildColorFromRGB(colorTuple.second[2]), buildColorFromRGB(colorTuple.second[3]), buildColorFromRGB(colorTuple.second[4]), buildColorFromRGB(colorTuple.second[5]));    
        }
        else
        {
            COMMONHEAD_LOG_WARN("error build color, type:" << static_cast<int>(colorTuple.first));
            return std::make_shared<Colors>(colorTuple.first, model::Color{});
        }
    });
    return uiColors;
}

model::Color ColorBuilder::buildColorFromRGB(const std::string& rgbStr)
{
    if (rgbStr.length() != 7 || rgbStr[0] != '#')
    {
        return {};
    }

    int r = hexPairToInt(rgbStr.substr(1, 2));
    if (-1 == r)
    {
        return {};
    }

    int g = hexPairToInt(rgbStr.substr(3, 2));
    if (-1 == g)
    {
        return {};
    }

    int b = hexPairToInt(rgbStr.substr(5, 2));
    if (-1 == b)
    {
        return {};
    }

    return model::Color{r, g, b};
}

int ColorBuilder::hexCharToInt(char c) 
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    else if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 10;
    }
    else if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }
    return -1;
}

int ColorBuilder::hexPairToInt(const std::string& hexPair) 
{
    if (hexPair.length() != 2)
    {
        return -1;
    }

    int high = hexCharToInt(hexPair[0]);
    if (-1 == high)
    {
        return -1;
    }

    int low = hexCharToInt(hexPair[1]);
    if (-1 == low)
    {
        return -1;
    }
    return high * 16 + low;
}
}