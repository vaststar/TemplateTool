#include <commonHead/viewModels/MediaCameraViewModel/CameraSource.h>

#include <type_traits>

namespace commonHead::viewModels::model {

std::string toDisplayString(const CameraSource& source)
{
    return std::visit(
        [](const auto& cameraSource) -> std::string {
            using SourceType = std::decay_t<decltype(cameraSource)>;
            if constexpr (std::is_same_v<SourceType, LocalCameraSource>)
            {
                return "local:" + std::to_string(cameraSource.index);
            }
            else
            {
                return "url:" + cameraSource.url;
            }
        },
        source);
}

} // namespace commonHead::viewModels::model
