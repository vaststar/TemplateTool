// CameraDirectoryServiceTypes holds the service's outward data structures.
// Those are currently pure-virtual entity interfaces (ICameraDirectoryEntities.h)
// which are a compile-time contract and emit no linkable symbols, so this
// exported anchor gives the shared library a symbol and an import library.
// Replace it with a real exported type once one gains an out-of-line body.
#include <ucf/Services/CameraDirectoryService/CameraDirectoryServiceTypesExport.h>

#include <ucf/Services/CameraDirectoryService/ICameraDirectoryEntities.h>

namespace ucf::service::model {

CAMERA_DIRECTORY_SERVICE_TYPES_API void cameraDirectoryTypesAnchor() {}

}
