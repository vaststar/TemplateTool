#pragma once

#include <string>
#include <variant>

#include <commonHead/viewModels/MediaCameraViewModel/MediaCameraViewModelTypesExport.h>

namespace commonHead::viewModels::model {

// VM-layer local camera description. It deliberately remains separate from
// the corresponding service-layer type to preserve the UI/VM/service boundary.
struct MEDIA_CAMERA_VIEW_MODEL_TYPES_API LocalCameraSource
{
    int index = 0;
};

struct MEDIA_CAMERA_VIEW_MODEL_TYPES_API NetworkCameraSource
{
    std::string url;
    std::string transport;
    int openTimeoutMs = 5000;
    int readTimeoutMs = 5000;
};

using CameraSource = std::variant<LocalCameraSource, NetworkCameraSource>;

MEDIA_CAMERA_VIEW_MODEL_TYPES_API std::string toDisplayString(const CameraSource& source);

} // namespace commonHead::viewModels::model
