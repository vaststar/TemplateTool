// MediaServiceTypes currently exposes header-only value types and the
// IVideoFrame contract. This anchor gives shared builds an exported symbol and
// an import library until a public type gains an out-of-line implementation.
#include <ucf/services/MediaService/MediaServiceTypesExport.h>

#include <ucf/services/MediaService/MediaTypes.h>

namespace ucf::service::media {

MEDIA_SERVICE_TYPES_API void mediaServiceTypesAnchor() {}

} // namespace ucf::service::media
