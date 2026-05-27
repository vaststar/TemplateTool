#include <commonHead/viewModels/MediaCameraViewModel/CameraSource.h>

namespace commonHead::viewModels::model {

std::string toDisplayString(const CameraSource& source)
{
    return std::visit([](const auto& s) -> std::string {
        using T = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<T, LocalCameraSource>)
            return "local:" + std::to_string(s.index);
        else
            return "url:" + s.url;
    }, source);
}

}
