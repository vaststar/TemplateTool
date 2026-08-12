#pragma once

#if defined(RECORDING_VIEW_MODEL_IMPL_STATIC) && defined(RECORDING_VIEW_MODEL_IMPL_SHARED)
#  error "RECORDING_VIEW_MODEL_IMPL_STATIC and RECORDING_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(RECORDING_VIEW_MODEL_IMPL_STATIC)
#    define RECORDING_VIEW_MODEL_IMPL_API
#  elif defined(RECORDING_VIEW_MODEL_IMPL_SHARED)
#    define RECORDING_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define RECORDING_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(RECORDING_VIEW_MODEL_IMPL_STATIC)
#    define RECORDING_VIEW_MODEL_IMPL_API
#  else
#    define RECORDING_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
