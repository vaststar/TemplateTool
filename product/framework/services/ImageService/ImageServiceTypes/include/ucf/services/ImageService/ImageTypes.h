#pragma once

#include <vector>

#include <ucf/services/ImageService/ImageServiceTypesExport.h>

namespace ucf::service::model{

using ImageBuffer = std::vector<unsigned char>;
struct IMAGE_SERVICE_TYPES_API Image
{
    Image(const ImageBuffer& buffer, int width, int height, int steps)
        : buffer(buffer)
        , width(width)
        , height(height)
        , steps(steps)
    {

    }

    // const members below make the type non-assignable. The assignment
    // operators are explicitly deleted so that dllexport does not try to
    // instantiate the implicitly-deleted ones (which would raise C2280).
    Image(const Image&) = default;
    Image(Image&&) = default;
    Image& operator=(const Image&) = delete;
    Image& operator=(Image&&) = delete;

    const ImageBuffer buffer;
    const int width;
    const int height;
    const int steps;
};

}
