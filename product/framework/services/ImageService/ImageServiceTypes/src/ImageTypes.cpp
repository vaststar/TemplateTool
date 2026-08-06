// Translation unit that materialises the exported value types of
// ImageServiceTypes. Including the header (which marks the types SERVICE_EXPORT)
// while this TU is compiled with SERVICE_DLL causes MSVC to emit and export the
// type symbols, so the shared library produces an import library.
#include <ucf/Services/ImageService/ImageTypes.h>
