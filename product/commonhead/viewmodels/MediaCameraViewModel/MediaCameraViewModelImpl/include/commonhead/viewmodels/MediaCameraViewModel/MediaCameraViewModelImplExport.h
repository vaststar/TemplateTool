#pragma once

#if defined(MEDIA_CAMERA_VIEW_MODEL_IMPL_STATIC) && defined(MEDIA_CAMERA_VIEW_MODEL_IMPL_SHARED)
#  error "MEDIA_CAMERA_VIEW_MODEL_IMPL_STATIC and MEDIA_CAMERA_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(MEDIA_CAMERA_VIEW_MODEL_IMPL_STATIC)
#    define MEDIA_CAMERA_VIEW_MODEL_IMPL_API
#  elif defined(MEDIA_CAMERA_VIEW_MODEL_IMPL_SHARED)
#    define MEDIA_CAMERA_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define MEDIA_CAMERA_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(MEDIA_CAMERA_VIEW_MODEL_IMPL_STATIC)
#    define MEDIA_CAMERA_VIEW_MODEL_IMPL_API
#  else
#    define MEDIA_CAMERA_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
