#pragma once

#if defined(CAMERA_DIRECTORY_VIEW_MODEL_IMPL_STATIC) && defined(CAMERA_DIRECTORY_VIEW_MODEL_IMPL_SHARED)
#  error "CAMERA_DIRECTORY_VIEW_MODEL_IMPL_STATIC and CAMERA_DIRECTORY_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(CAMERA_DIRECTORY_VIEW_MODEL_IMPL_STATIC)
#    define CAMERA_DIRECTORY_VIEW_MODEL_IMPL_API
#  elif defined(CAMERA_DIRECTORY_VIEW_MODEL_IMPL_SHARED)
#    define CAMERA_DIRECTORY_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define CAMERA_DIRECTORY_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(CAMERA_DIRECTORY_VIEW_MODEL_IMPL_STATIC)
#    define CAMERA_DIRECTORY_VIEW_MODEL_IMPL_API
#  else
#    define CAMERA_DIRECTORY_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
