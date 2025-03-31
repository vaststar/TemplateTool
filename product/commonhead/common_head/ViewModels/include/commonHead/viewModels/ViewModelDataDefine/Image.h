#pragma once

#include <vector>
#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>

namespace commonHead::viewModels::model{

using ImageBuffer = std::vector<unsigned char>;
struct COMMONHEAD_EXPORT Image
{
    Image(const ImageBuffer& buffer, int width, int height, int steps)
        : buffer(buffer)
        , width(width)
        , height(height)
        , steps(steps)
    {

    }
    const ImageBuffer buffer;
    const int width;
    const int height;
    const int steps;
};
}