#pragma once

#if defined(SIDE_BAR_VIEW_MODEL_IMPL_STATIC) && defined(SIDE_BAR_VIEW_MODEL_IMPL_SHARED)
#  error "SIDE_BAR_VIEW_MODEL_IMPL_STATIC and SIDE_BAR_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(SIDE_BAR_VIEW_MODEL_IMPL_STATIC)
#    define SIDE_BAR_VIEW_MODEL_IMPL_API
#  elif defined(SIDE_BAR_VIEW_MODEL_IMPL_SHARED)
#    define SIDE_BAR_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define SIDE_BAR_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(SIDE_BAR_VIEW_MODEL_IMPL_STATIC)
#    define SIDE_BAR_VIEW_MODEL_IMPL_API
#  else
#    define SIDE_BAR_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
