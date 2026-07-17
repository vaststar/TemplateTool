#pragma once

// FFmpegUtils is part of the Utilities shared library, so it reuses the shared
// Utilities export macro. A dedicated header is kept so that, if FFmpegUtils is
// ever split into its own DLL, only this file needs to change.
#include <ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h>

#define FFmpegUtils_EXPORT Utilities_EXPORT
