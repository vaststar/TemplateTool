// Translation unit that materialises the exported value types of
// ImageServiceTypes. Including the header (which marks the types
// IMAGE_SERVICE_TYPES_EXPORT) while this TU is compiled with
// IMAGE_SERVICE_TYPES_DLL causes MSVC to emit and export the type symbols, so
// the shared library produces an import library.
#include <ucf/Services/ImageService/ImageTypes.h>
