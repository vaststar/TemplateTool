#pragma once

#if defined(MEDIA_CAMERA_VIEW_MODEL_TYPES_STATIC) && defined(MEDIA_CAMERA_VIEW_MODEL_TYPES_SHARED)
#  error "MEDIA_CAMERA_VIEW_MODEL_TYPES_STATIC and MEDIA_CAMERA_VIEW_MODEL_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(MEDIA_CAMERA_VIEW_MODEL_TYPES_STATIC)
#    define MEDIA_CAMERA_VIEW_MODEL_TYPES_API
#  elif defined(MEDIA_CAMERA_VIEW_MODEL_TYPES_SHARED)
#    define MEDIA_CAMERA_VIEW_MODEL_TYPES_API __declspec(dllexport)
#  else
#    define MEDIA_CAMERA_VIEW_MODEL_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(MEDIA_CAMERA_VIEW_MODEL_TYPES_STATIC)
#    define MEDIA_CAMERA_VIEW_MODEL_TYPES_API
#  else
#    define MEDIA_CAMERA_VIEW_MODEL_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
