#pragma once

#if defined(STABILITY_VIEW_MODEL_IMPL_STATIC) && defined(STABILITY_VIEW_MODEL_IMPL_SHARED)
#  error "STABILITY_VIEW_MODEL_IMPL_STATIC and STABILITY_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(STABILITY_VIEW_MODEL_IMPL_STATIC)
#    define STABILITY_VIEW_MODEL_IMPL_API
#  elif defined(STABILITY_VIEW_MODEL_IMPL_SHARED)
#    define STABILITY_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define STABILITY_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(STABILITY_VIEW_MODEL_IMPL_STATIC)
#    define STABILITY_VIEW_MODEL_IMPL_API
#  else
#    define STABILITY_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
