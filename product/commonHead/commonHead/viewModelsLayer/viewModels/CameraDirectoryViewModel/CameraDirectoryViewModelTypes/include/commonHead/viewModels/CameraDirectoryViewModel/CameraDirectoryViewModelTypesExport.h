#pragma once

#if defined(CAMERA_DIRECTORY_VIEW_MODEL_TYPES_STATIC) && defined(CAMERA_DIRECTORY_VIEW_MODEL_TYPES_SHARED)
#  error "CAMERA_DIRECTORY_VIEW_MODEL_TYPES_STATIC and CAMERA_DIRECTORY_VIEW_MODEL_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(CAMERA_DIRECTORY_VIEW_MODEL_TYPES_STATIC)
#    define CAMERA_DIRECTORY_VIEW_MODEL_TYPES_API
#  elif defined(CAMERA_DIRECTORY_VIEW_MODEL_TYPES_SHARED)
#    define CAMERA_DIRECTORY_VIEW_MODEL_TYPES_API __declspec(dllexport)
#  else
#    define CAMERA_DIRECTORY_VIEW_MODEL_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(CAMERA_DIRECTORY_VIEW_MODEL_TYPES_STATIC)
#    define CAMERA_DIRECTORY_VIEW_MODEL_TYPES_API
#  else
#    define CAMERA_DIRECTORY_VIEW_MODEL_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
