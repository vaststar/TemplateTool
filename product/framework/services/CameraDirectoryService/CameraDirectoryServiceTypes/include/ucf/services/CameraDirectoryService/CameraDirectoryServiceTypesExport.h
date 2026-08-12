#pragma once

// Per-module export macro for CameraDirectoryServiceTypes.
//   CAMERA_DIRECTORY_SERVICE_TYPES_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   CAMERA_DIRECTORY_SERVICE_TYPES_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(CAMERA_DIRECTORY_SERVICE_TYPES_STATIC) && defined(CAMERA_DIRECTORY_SERVICE_TYPES_SHARED)
#  error "CAMERA_DIRECTORY_SERVICE_TYPES_STATIC and CAMERA_DIRECTORY_SERVICE_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(CAMERA_DIRECTORY_SERVICE_TYPES_STATIC)
#    define CAMERA_DIRECTORY_SERVICE_TYPES_API
#  elif defined(CAMERA_DIRECTORY_SERVICE_TYPES_SHARED)
#    define CAMERA_DIRECTORY_SERVICE_TYPES_API __declspec(dllexport)
#  else
#    define CAMERA_DIRECTORY_SERVICE_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(CAMERA_DIRECTORY_SERVICE_TYPES_STATIC)
#    define CAMERA_DIRECTORY_SERVICE_TYPES_API
#  else
#    define CAMERA_DIRECTORY_SERVICE_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
