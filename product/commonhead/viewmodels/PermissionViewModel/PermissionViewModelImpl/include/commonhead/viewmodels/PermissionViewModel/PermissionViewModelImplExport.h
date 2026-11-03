#pragma once

#if defined(PERMISSION_VIEW_MODEL_IMPL_STATIC) && defined(PERMISSION_VIEW_MODEL_IMPL_SHARED)
#  error "PERMISSION_VIEW_MODEL_IMPL_STATIC and PERMISSION_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(PERMISSION_VIEW_MODEL_IMPL_STATIC)
#    define PERMISSION_VIEW_MODEL_IMPL_API
#  elif defined(PERMISSION_VIEW_MODEL_IMPL_SHARED)
#    define PERMISSION_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define PERMISSION_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(PERMISSION_VIEW_MODEL_IMPL_STATIC)
#    define PERMISSION_VIEW_MODEL_IMPL_API
#  else
#    define PERMISSION_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
