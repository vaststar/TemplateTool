#pragma once

#if defined(SIDE_BAR_VIEW_MODEL_TYPES_STATIC) && defined(SIDE_BAR_VIEW_MODEL_TYPES_SHARED)
#  error "SIDE_BAR_VIEW_MODEL_TYPES_STATIC and SIDE_BAR_VIEW_MODEL_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(SIDE_BAR_VIEW_MODEL_TYPES_STATIC)
#    define SIDE_BAR_VIEW_MODEL_TYPES_API
#  elif defined(SIDE_BAR_VIEW_MODEL_TYPES_SHARED)
#    define SIDE_BAR_VIEW_MODEL_TYPES_API __declspec(dllexport)
#  else
#    define SIDE_BAR_VIEW_MODEL_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(SIDE_BAR_VIEW_MODEL_TYPES_STATIC)
#    define SIDE_BAR_VIEW_MODEL_TYPES_API
#  else
#    define SIDE_BAR_VIEW_MODEL_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
