#pragma once

#if defined(RECORDING_VIEW_MODEL_TYPES_STATIC) && defined(RECORDING_VIEW_MODEL_TYPES_SHARED)
#  error "RECORDING_VIEW_MODEL_TYPES_STATIC and RECORDING_VIEW_MODEL_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(RECORDING_VIEW_MODEL_TYPES_STATIC)
#    define RECORDING_VIEW_MODEL_TYPES_API
#  elif defined(RECORDING_VIEW_MODEL_TYPES_SHARED)
#    define RECORDING_VIEW_MODEL_TYPES_API __declspec(dllexport)
#  else
#    define RECORDING_VIEW_MODEL_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(RECORDING_VIEW_MODEL_TYPES_STATIC)
#    define RECORDING_VIEW_MODEL_TYPES_API
#  else
#    define RECORDING_VIEW_MODEL_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
